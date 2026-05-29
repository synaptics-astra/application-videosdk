import QtQuick 2.2
import QtQuick.Window 2.1
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.1
import QtQuick.Dialogs

Item {
    width: parent.width
    height: parent.height
    z: 10
    property string currentDirectory: "/home/root/demos/videos/h264"
    property string selectedFilePath: ""
    property bool fileDialogVisible: false

    function resolvePath(basePath, relativePath) {
        var baseParts = basePath.split('/');
        var relativeParts = relativePath.split('/');

        for (var i = 0; i < relativeParts.length; i++) {
            var part = relativeParts[i];

            if (part === '..') {
                baseParts.pop();
            } else if (part === '.') {
                continue;
            } else {
                baseParts.push(part);
            }
        }

        var resolvedPath = baseParts.join('/');
        if (resolvedPath.endsWith('/')) {
            resolvedPath = resolvedPath.slice(0, -1);
        }
        if (resolvedPath.slice(0, 2) === "//") {
            resolvedPath = resolvedPath.slice(1);
        }
        if(resolvedPath === "")
        {
            resolvedPath = "/"
        }
        return resolvedPath;
    }

    signal updateProperty(value: string)

    Rectangle {
        id: fileDialog
        width: parent.width
        height: parent.height
        color: "#ffffff"
        border.color: "#000000"
        radius: 12
        anchors.centerIn: parent
        anchors.fill: parent
        visible: fileDialogVisible
        z: 10

        Button {
            text: "Cancel"
            width: parent.width * 0.12
            height: 45
            font.pixelSize: 16
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.margins: 10
            highlighted : true
            background: Rectangle {
                color: "#444444"
                radius: 10
            }
            onClicked: {
                fileDialogVisible = false
            }
        }

        Rectangle {
            anchors.fill: parent
            color: "transparent"
            radius: 12
            border.color: "#aaa"
            border.width: 1
            anchors.centerIn: parent
            z: -1
        }

        Rectangle {
            anchors.fill: parent
            color: Qt.rgba(0, 0, 0, 0.5)
            z: -2
        }

        ListView {
            id: fileListView
            width: parent.width
            height: parent.height - (parent.height * 0.2)
            model: ListModel {}
            anchors.centerIn: parent

            delegate: Item {
                width: fileListView.width
                height: 45

                Rectangle {
                    width: parent.width
                    height: 45
                    color: model.isFolder ? "#d1e8e2" : "#f0f0f0"
                    border.color: "#cccccc"
                    radius: 5
                    anchors.margins: 5

                    Text {
                        anchors.centerIn: parent
                        text: model.name
                        font.pixelSize: 16
                        color: model.isFolder ? "black" : "#0077cc"
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (model.isFolder) {
                                if(model.name == "../") {
                                    currentDirectory = resolvePath(currentDirectory, "../")
                                }
                                else {
                                    currentDirectory = resolvePath(currentDirectory, model.name)
                                }
                                updateFileList()
                            } else {
                                selectedFilePath = currentDirectory + "/" + model.name
                                fileDialogVisible = false
                                updateProperty(selectedFilePath)
                            }
                        }
                    }
                }
            }
        }
    }

    function updateFileList() {
        var files = fileDialogHelper.getFilesFromDirectory(currentDirectory, "h264");
        fileListView.model.clear();

        if(currentDirectory != "/") {
            fileListView.model.append({name: "../", isFolder: true});
        }

        for (var i = 0; i < files.length; i++) {
            var isFolder = fileDialogHelper.isDirectory(currentDirectory + "/" + files[i]);
            fileListView.model.append({name: files[i], isFolder: isFolder});
        }
    }
}
