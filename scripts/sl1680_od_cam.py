"""
SPDX-License-Identifier: Apache-2.0
Copyright 2026 Synaptics Incorporated
GStreamer camera pipeline with OpenCV overlay
"""


import argparse
import glob
import json
import os
import re
import sys
import time

try:
    import cv2
    import numpy as np
except ImportError:
    print("Error: OpenCV (cv2) and numpy required.", file=sys.stderr)
    sys.exit(1)

try:
    import gi
    gi.require_version("Gst", "1.0")
    from gi.repository import Gst
except (ImportError, ValueError) as e:
    print(f"Error: GStreamer required. {e}", file=sys.stderr)
    sys.exit(1)

try:
    from synap import Network
    from synap.preprocessor import Preprocessor
    from synap.postprocessor import Detector
    from synap.types import Layout, Shape
except ImportError as e:
    print(f"Error: synap library required. {e}", file=sys.stderr)
    sys.exit(1)


DEFAULT_MODEL_PATH = "/usr/share/synap/models/object_detection/coco/model/yolov8s-640x384/model.synap"
DEFAULT_LABELS_PATH = "/usr/share/synap/models/object_detection/coco/info.json"


def _video_sort_key(path: str) -> int:
    match = re.search(r"(\d+)$", path)
    return int(match.group(1)) if match else 10**9


def _find_usb_camera_from_v4l2ctl() -> str | None:
    try:
        import subprocess
        result = subprocess.run(
            ["v4l2-ctl", "--list-devices"],
            capture_output=True,
            text=True,
            timeout=2,
        )
        if result.returncode != 0:
            return None

        lines = result.stdout.splitlines()
        in_usb_device = False
        for line in lines:
            if "usb-" in line.lower():
                in_usb_device = True
                continue
            if in_usb_device and "/dev/video" in line:
                device = line.strip()
                if os.path.exists(device):
                    return device
            if line.strip() == "":
                in_usb_device = False
    except Exception:
        return None

    return None


def auto_select_camera() -> str:
    usb_camera = _find_usb_camera_from_v4l2ctl()
    if usb_camera is not None:
        return usb_camera

    devices = sorted(glob.glob("/dev/video*"), key=_video_sort_key)
    if not devices:
        raise FileNotFoundError("No camera device found under /dev/video*.")
    return devices[0]


def load_labels(labels_path: str) -> dict | None:
    """Load COCO labels from JSON file."""
    try:
        if os.path.exists(labels_path):
            data = json.loads(open(labels_path).read())
            labels_list = data.get("labels", [])
            # Convert list to dict with index as key
            return {i: label for i, label in enumerate(labels_list)}
    except Exception as e:
        print(f"Warning: Failed to load labels: {e}", file=sys.stderr)
    return None


class CameraInferencePipeline:
    """GStreamer camera pipeline with OpenCV overlay."""

    def __init__(
        self,
        camera_device: str,
        model_path: str,
        labels_path: str,
        score_threshold: float = 0.5,
        max_detections: int = 0,
    ):
        self.camera_device = camera_device
        self.model_path = model_path
        self.pipeline = None
        self.appsink = None
        self.frame_count = 0
        self.running = False
        self.frame_width = 640
        self.frame_height = 384

        # Load labels
        self.labels = load_labels(labels_path)

        # Initialize synap inference
        try:
            self.network = Network(model_path)
            self.preprocessor = Preprocessor()
            self.detector = Detector(
                score_threshold=score_threshold,
                n_max=max_detections,  # Note: parameter name is 'n_max' not 'max_detections'
                nms=True,  # Note: parameter name is 'nms' not 'enable_nms'
                iou_threshold=0.5,
                iou_with_min=False
            )
        except Exception as e:
            raise RuntimeError(f"Failed to initialize synap: {e}")

        Gst.init(None)
        self._build_pipeline()

    def _build_pipeline(self) -> None:
        """Build GStreamer pipeline (without synapinfer element)."""
        pipeline_str = (
            f"v4l2src device={self.camera_device} "
            f"! video/x-raw,framerate=30/1,format=YUY2,width=640,height=480 "
            f"! synavideoconvertscale ! video/x-raw,format=RGB,width={self.frame_width},height={self.frame_height} "
            f"! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream  ! appsink name=sink emit-signals=true  "
        )

        print(f"Pipeline: {pipeline_str}\n")

        try:
            self.pipeline = Gst.parse_launch(pipeline_str)
        except Exception as e:
            raise RuntimeError(f"Failed to parse pipeline: {e}")

        self.appsink = self.pipeline.get_by_name("sink")
        if not self.appsink:
            raise RuntimeError("Failed to get appsink")

        self.appsink.connect("new-sample", self._on_new_sample)

    def _run_inference(self, frame: np.ndarray) -> dict | None:
        """Run inference on frame using synap.Network."""
        try:
            # Frame is (384, 640, 3), but tensor expects (1, 384, 640, 3) with batch dimension
            frame_with_batch = np.expand_dims(frame, axis=0)

            # Use correct Layout enum value: nhwc
            from synap.types import Layout
            assigned_rect = self.preprocessor.assign(
                self.network.inputs,
                frame_with_batch,
                layout=Layout.nhwc
            )

            # Run inference
            outputs = self.network.predict()

            # Post-process results
            result = self.detector.process(outputs, assigned_rect)

            return result
        except Exception as e:
            print(f"Error during inference: {e}", file=sys.stderr)
            return None

    def _draw_detections(self, frame: np.ndarray, result) -> np.ndarray:
        """Draw detections on frame."""
        if not result or not hasattr(result, 'items'):
            return frame

        for item in result.items:
            bb = item.bounding_box

            # Bounding box coordinates
            x1 = int(bb.origin.x)
            y1 = int(bb.origin.y)
            x2 = int(bb.origin.x + bb.size.x)
            y2 = int(bb.origin.y + bb.size.y)

            # Draw box
            cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 0), 2)

            # Label with class and confidence
            class_idx = item.class_index
            confidence = item.confidence

            label = f"Class {class_idx}"
            if self.labels and class_idx in self.labels:
                label = self.labels[class_idx]

            text = f"{label} {confidence:.2f}"
            cv2.putText(
                frame, text, (x1, y1 - 10),
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2
            )

        return frame

    def _on_new_sample(self, appsink) -> int:
        """Callback for new sample from appsink."""
        try:
            sample = appsink.emit("pull-sample")
            if sample:
                buffer = sample.get_buffer()
                caps = sample.get_caps()

                struct = caps.get_structure(0)
                width = struct.get_int("width")[1]
                height = struct.get_int("height")[1]

                # Extract frame data
                data = buffer.extract_dup(0, buffer.get_size())
                frame = np.frombuffer(data, dtype=np.uint8).reshape((height, width, 3))
                frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)
                # Ensure frame is contiguous in memory and has proper dtype
                frame = np.ascontiguousarray(frame, dtype=np.uint8)

                # Run inference
                result = self._run_inference(frame)

                # Draw detections
                frame = self._draw_detections(frame, result)

                self.frame_count += 1

                try:
                    try:
                        if cv2.getWindowProperty("Object Detection", cv2.WND_PROP_VISIBLE) < 1:
                            self.running = False
                            return Gst.FlowReturn.OK
                    except cv2.error:
                        pass
                    cv2.imshow("Object Detection", frame)
                    key = cv2.waitKey(1) & 0xFF
                    if key == ord("q") or key == 27:
                        self.running = False
                except cv2.error as e:
                    if "function is not implemented" not in str(e):
                        self.running = False

        except Exception as e:
            print(f"Error processing sample: {e}", file=sys.stderr)

        return Gst.FlowReturn.OK

    def start(self) -> None:
        """Start pipeline."""
        ret = self.pipeline.set_state(Gst.State.PLAYING)
        if ret == Gst.StateChangeReturn.FAILURE:
            raise RuntimeError("Failed to set pipeline to PLAYING")
        self.running = True
        time.sleep(0.5)

    def run(self) -> None:
        """Run pipeline until stopped."""
        self.start()
        last_stat = 0.0
        start_time = time.time()

        try:
            while self.running:
                time.sleep(0.1)
                now = time.time()
                if now - last_stat >= 0.5:
                    elapsed = now - start_time
                    fps = self.frame_count / elapsed if elapsed > 0 else 0
                    print(f"Frame: {self.frame_count:6d} | FPS: {fps:8.1f}", flush=True)
                    last_stat = now

        except KeyboardInterrupt:
            print("Stopped by user.")
        finally:
            self.stop()

    def stop(self) -> None:
        """Stop pipeline."""
        self.running = False
        if self.pipeline:
            self.pipeline.set_state(Gst.State.NULL)
        cv2.destroyAllWindows()


def main() -> int:
    parser = argparse.ArgumentParser(description="Camera inference with synap.Network and OpenCV overlay")
    parser.add_argument("--camera", default="auto", help="Camera device (default: auto-detect)")
    parser.add_argument("--model", default=DEFAULT_MODEL_PATH, help="Model path")
    parser.add_argument("--labels", default=DEFAULT_LABELS_PATH, help="Labels JSON path")
    parser.add_argument("--score-threshold", type=float, default=0.5, help="Detection confidence threshold")
    parser.add_argument("--max-detections", type=int, default=0, help="Max detections (0=all)")

    args = parser.parse_args()

    if args.camera.lower() == "auto":
        try:
            camera_device = auto_select_camera()
            print(f"Auto-selected camera: {camera_device}\n")
        except FileNotFoundError as e:
            print(f"Error: {e}", file=sys.stderr)
            return 1
    else:
        camera_device = args.camera

    if not os.path.exists(camera_device):
        print(f"Error: camera device not found: {camera_device}", file=sys.stderr)
        return 1

    if not os.path.exists(args.model):
        print(f"Error: model not found: {args.model}", file=sys.stderr)
        return 1

    print(f"Camera: {camera_device}")
    print(f"Model: {args.model}")
    print(f"Labels: {args.labels}\n")

    try:
        pipeline = CameraInferencePipeline(
            camera_device,
            args.model,
            args.labels,
            score_threshold=args.score_threshold,
            max_detections=args.max_detections
        )
        pipeline.run()
        print(f"\nTotal frames: {pipeline.frame_count}")
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
