import QtQuick 2.0 as QQ2
import Qt3D.Core 2.15
import Qt3D.Render 2.15
import Qt3D.Logic 2.15
import Qt3D.Input 2.15
import Qt3D.Extras 2.15
import QtQuick.Scene3D 2.15

Scene3D {
	Entity {
		id: sceneRoot

		Camera {
			id: camera
			projectionType: CameraLens.PerspectiveProjection
			fieldOfView: 45
			aspectRatio: 4/3
			nearPlane : 0.1
			farPlane : 1000.0
			position: Qt.vector3d( 0.0, 0.0, -4.0 )
			upVector: Qt.vector3d( 0.0, 1.0, 0.0 )
			viewCenter: Qt.vector3d( 0.0, 0.0, 0.0 )
		}

		OrbitCameraController  {
			camera: camera
		}

		components: [
			RenderSettings{
				activeFrameGraph: ForwardRenderer {
					clearColor: Qt.rgba(0.0, 0.0, 0.0, 0.0)
					camera: camera
				}
			}
		]

		CuboidMesh {
			id: sphereMesh
			yzMeshResolution: Qt.size(10, 10)
			xzMeshResolution: Qt.size(10, 10)
			xyMeshResolution: Qt.size(10, 10)
		}

		TextureMaterial {
			id: material
			texture: offscreenTexture
		}

		Texture2D {
			id: offscreenTexture
			generateMipMaps: true
			magnificationFilter: Texture.Linear
			minificationFilter: Texture.LinearMipMapLinear
			wrapMode {
				x: WrapMode.Repeat
				y: WrapMode.Repeat
			}
			TextureImage {
				id: diffuseTextureImage
				source: "SynaAVeda.jpg"
			}
		}

		Transform {
			id: sphereTransform
			property real userAngle: 0.0
			matrix: {
				var a = Qt.matrix4x4();
				var b = 1.7;
				var m = a.times(b);
				m.rotate(userAngle, Qt.vector3d(0, 1, 0));
				m.rotate(userAngle, Qt.vector3d(1, 0, 0));
				m.rotate(userAngle, Qt.vector3d(0, 0, 1));
				//m.translate(Qt.vector3d(0, 0, 0));
				return m;
			}
		}

		Entity {
			id: sphereEntity
			components: [ sphereMesh, material, sphereTransform ]
		}

		QQ2.NumberAnimation {
			target: sphereTransform
			property: "userAngle"
			duration: 18000
			from: 0
			to: 360
			loops: QQ2.Animation.Infinite
			running: true
		}
	}
}
