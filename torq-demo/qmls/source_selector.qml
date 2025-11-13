import QtQuick 2.2

Item {
    // Object Detection: NPU pipeline
    property string objectDetectionNPU: "CAMSRC ! tee name=t_data t_data. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream ! synavideoconvertscale ! video/x-raw,width=320,height=320,format=RGB ! synapinfer model=/usr/share/synap/models/object_detection/coco/npu/model.synap mode=detector frameinterval=20 ! overlay.inference_sink t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=480,format=BGRA ! synapoverlay name=overlay label=/usr/share/synap/models/object_detection/coco/info.json ! placeholdersink"

    // Object Detection: CPU pipeline
    property string objectDetectionCPU: "CAMSRC ! tee name=t_data t_data. ! queue max-size-buffers=1 max-size-bytes=0 max-size-time=0 leaky=downstream ! synavideoconvertscale ! video/x-raw,width=320,height=320,format=RGB ! synapinfer model=/usr/share/synap/models/object_detection/coco/cpu/model.synap mode=detector frameinterval=20 ! overlay.inference_sink t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=480,format=BGRA ! synapoverlay name=overlay label=/usr/share/synap/models/object_detection/coco/info.json ! placeholdersink"

    // Image Classification: NPU pipeline
    property string imageClassificationNPU: "filesrc location=/home/root/demos/videos/mp4/image_classification.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! tee name=t_data t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=224,height=224,format=RGB ! synapinfer model=/usr/share/synap/models/image_classification/mobilenetv2/npu/model.synap mode=classifier frameinterval=3 threshold=0.30 ! overlay.inference_sink t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=480,format=BGRA ! synapoverlay name=overlay label=/usr/share/synap/models/image_classification/mobilenetv2/info.json ! placeholdersink"

    // Image Classification: CPU pipeline
    property string imageClassificationCPU: "filesrc location=/home/root/demos/videos/mp4/image_classification.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! tee name=t_data t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=224,height=224,format=RGB ! synapinfer model=/usr/share/synap/models/image_classification/mobilenetv2/cpu/model.synap mode=classifier frameinterval=3 threshold=0.30 ! overlay.inference_sink t_data. ! queue ! synavideoconvertscale ! video/x-raw,width=640,height=480,format=BGRA ! synapoverlay name=overlay label=/usr/share/synap/models/image_classification/mobilenetv2/info.json ! placeholdersink"
}
