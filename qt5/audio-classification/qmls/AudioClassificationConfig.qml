import QtQuick 2.15

QtObject {
    property real confidenceThreshold: 0.5
    property string modelPath: "/usr/share/synap/models/audio_classification/yamnet/model/model.tflite"
    property string labelPath: "/usr/share/synap/models/audio_classification/yamnet/info.txt"
    property string gstPipeline: "alsasrc device=hw:1,0 ! audio/x-raw,format=S16LE,rate=16000,channels=2 ! decodebin ! audioconvert ! audioresample ! capsfilter caps=audio/x-raw,format=S16LE,channels=1,rate=16000,layout=interleaved ! tensor_converter frames-per-tensor=3900 ! tensor_aggregator frames-in=3900 frames-out=15600 frames-flush=3900 frames-dim=1 ! tensor_transform mode=arithmetic option=typecast:float32,add:0.5,div:32767.5 ! tensor_transform mode=transpose option=1:0:2:3 ! queue leaky=2 max-size-buffers=10 ! tensor_filter framework=tensorflow2-lite model=MODEL_PATH custom=Delegate:External,ExtDelegateLib:libvx_delegate.so ! appsink name=mysink"
}
