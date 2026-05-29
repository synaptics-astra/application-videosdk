import QtQuick 2.12

ListModel {
    ListElement { text: "Astra Intro"; value: "filesrc location=/home/root/demos/videos/mp4/astra_intro.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! placeholdersink"}
    ListElement { text: "Astra Building Blocks"; value: "filesrc location=/home/root/demos/videos/mp4/astra_building_blocks.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! placeholdersink"}
    ListElement { text: "Astra Edge AI"; value: "filesrc location=/home/root/demos/videos/mp4/astra_edge_ai.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! placeholdersink"}
    ListElement { text: "Astra Unboxing"; value: "filesrc location=/home/root/demos/videos/mp4/astra_unboxing.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! placeholdersink"}
    ListElement { text: "Astra Multimedia"; value: "filesrc location=/home/root/demos/videos/mp4/astra_multimedia.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! placeholdersink"}
    ListElement { text: "Astra Wireless"; value: "filesrc location=/home/root/demos/videos/mp4/astra_wireless.mp4 ! qtdemux name=demux demux.video_0 ! queue ! h264parse ! avdec_h264 ! placeholdersink"}
    ListElement { text: "Camera Source"; value: "CAMSRC ! placeholdersink"}
    ListElement { text: "RTSP"; value: "rtspsrc location=RTSPURL latency=100 ! rtpjitterbuffer ! rtph264depay wait-for-keyframe=true ! video/x-h264, width=1920, height=1080 ! h264parse ! avdec_h264 ! placeholdersink"}
}
