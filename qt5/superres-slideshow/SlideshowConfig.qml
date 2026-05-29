import QtQuick 2.15

QtObject {
    property string modelPath: "/usr/share/synap/models/image_processing/super_resolution/model/sr_fast_y_uv_1280x720_3840x2160/model.synap"
    property string gstPipeline: "filesrc location=PNG_FILE name=filesrc ! decodebin ! synavideoconvertscale ! video/x-raw, width=1280, height=720, format=NV12 ! tee name=t_data t_data. ! queue ! synapimageproc model=MODEL_PATH ! video/x-raw, width=3840, height=2160 ! synavideoconvertscale ! video/x-raw, format=RGB ! appsink name=srSink t_data. ! queue ! synavideoconvertscale ! video/x-raw, format=RGB ! appsink name=normalSink"
}
