import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import Resource 1.0

import "../custom_views"
import "../panels"
import "../elements"

BusyIndicator {
    id: loadingIndicator
    visible: false
    running: visible

    property color color: Resource.whiteColor
    property int size: 48

    implicitWidth: size
    implicitHeight: size

    contentItem: Item {
        implicitWidth: size
        implicitHeight: size

        RotationAnimator on rotation {
            from: 0
            to: 360
            duration: 1000
            loops: Animation.Infinite
            running: loadingIndicator.running
        }

        Canvas {
            anchors.fill: parent
            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                ctx.beginPath()
                ctx.arc(width / 2, height / 2, width / 2 - 3, 0, Math.PI * 1.5)
                ctx.lineWidth = 3
                ctx.strokeStyle = color
                ctx.stroke()
            }
        }
    }
}
