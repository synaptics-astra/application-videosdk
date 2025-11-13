pragma Singleton

import QtQuick 2.15

QtObject {
    property var pageMappings: ({
        "sl2619": {
            "default": {
                pages: [
                    { index: 0, component: "gettingStarted" },
                    { index: 1, component: "videoDemos2619" },
                    { index: 2, component: "gfxDemos" },
                    { index: 3, component: "aiDemos2619" },
                    { index: 4, component: "capabilityDemos" },
                    { index: 5, component: "realTimeStreaming" }
                ],
                menu: ["Getting Started", "Video", "Graphics", "AI", "Capability Demo", "Real Time Streaming"]
            }
        },
        "sl1620": {
            "default": {
                pages: [
                    { index: 0, component: "gettingStarted" },
                    { index: 1, component: "videoDemos1620" },
                    { index: 2, component: "gfxDemos" },
                    { index: 3, component: "aiDemos" },
                    { index: 4, component: "capabilityDemos" },
                    { index: 5, component: "realTimeStreaming" }
                ],
                menu: ["Getting Started", "Video", "Graphics", "AI", "Capability Demo", "Real Time Streaming"]
            },
            "x11": {
                pages: [
                    { index: 0, component: "gettingStarted" },
                    { index: 1, component: "videoDemos1620" },
                    { index: 2, component: "gfxDemos" },
                    { index: 3, component: "aiDemos" },
                    { index: 4, component: "realTimeStreaming" }
                ],
                menu: ["Getting Started", "Video", "Graphics", "AI", "Real Time Streaming"]
            }
        },
        "sl1640": {
            "default": {
                pages: [
                    { index: 0, component: "gettingStarted" },
                    { index: 1, component: "videoDemos1640" },
                    { index: 2, component: "gfxDemos" },
                    { index: 3, component: "aiDemos" },
                    { index: 4, component: "capabilityDemos" },
                    { index: 5, component: "realTimeStreaming" }
                ],
                menu: ["Getting Started", "Video", "Graphics", "AI", "Capability Demo", "Real Time Streaming"]
            },
            "x11": {
                pages: [
                    { index: 0, component: "gettingStarted" },
                    { index: 1, component: "videoDemos1640" },
                    { index: 2, component: "gfxDemos" },
                    { index: 3, component: "aiDemos" },
                    { index: 4, component: "realTimeStreaming" }
                ],
                menu: ["Getting Started", "Video", "Graphics", "AI", "Real Time Streaming"]
            }
        },
        "sl1680": {
            "default": {
                pages: [
                    { index: 0, component: "gettingStarted" },
                    { index: 1, component: "videoDemos1680" },
                    { index: 2, component: "gfxDemos" },
                    { index: 3, component: "aiDemos" },
                    { index: 4, component: "capabilityDemos" },
                    { index: 5, component: "realTimeStreaming" },
                    { index: 6, component: "cameraApp" }
                ],
                menu: ["Getting Started", "Video", "Graphics", "AI", "Capability Demo", "Real Time Streaming", "ISP Camera"]
            },
            "x11": {
                pages: [
                    { index: 0, component: "gettingStarted" },
                    { index: 1, component: "videoDemos1680" },
                    { index: 2, component: "gfxDemos" },
                    { index: 3, component: "aiDemos" },
                    { index: 4, component: "realTimeStreaming" }
                ],
                menu: ["Getting Started", "Video", "Graphics", "AI", "Real Time Streaming"]
            }
        }
    })

    function getComponent(machine, index, isX11) {
        var mode = isX11 ? "x11" : "default"
        var mappings = pageMappings[machine] ? pageMappings[machine][mode].pages : pageMappings["sl1680"][mode].pages
        for (var i = 0; i < mappings.length; i++) {
            if (mappings[i].index === index) {
                return mappings[i].component
            }
        }
        return null
    }

    function getMenuModel(machine, isX11) {
        var mode = isX11 ? "x11" : "default"
        return pageMappings[machine] ? pageMappings[machine][mode].menu : pageMappings["sl1680"][mode].menu
    }
}
