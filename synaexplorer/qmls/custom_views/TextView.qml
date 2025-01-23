import QtQuick 2.15
import QtQuick.Controls 2.5
import Resource 1.0

Label {
    property bool isBold: false
    property bool isHighlight: false
    clip: true
    color: isHighlight ? Resource.highlightColor : Resource.baseColor
    font.pixelSize: Resource.h3
    font.capitalization: Font.Capitalize
    font.bold: isBold ? Font.Bold : Font.Normal
    font.weight: isBold ? Font.Bold : Font.Normal
    font.family: Resource.vollkornRegular.name
    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
}
