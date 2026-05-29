import QtQuick 2.15
import QtQuick.Controls 2.5
import Resource 1.0

Label {
    property bool isBold: false
    property bool isHighlight: false
    clip: true
    color: "#000000"
    font.pixelSize: Resource.h3 ? Resource.h3 : 16
    font.capitalization: Font.Capitalize
    font.weight: isBold ? Font.Bold : Font.Normal
    font.family: Resource.vollkornRegular ? Resource.vollkornRegular.name : "Arial"
    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
}