#ifndef EGL_EGLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES 1
#endif

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#include "demo_videoitem.h"
#include "demo_videostream.h"
#include "logs.h"

#include <QtCore/QTime>
#include <QtCore/QThread>
#include <QtQuick/QSGTextureProvider>
#include <QtGui/QOpenGLFunctions>

extern void updateCounter();
extern int fVisible[];
extern PlatformType platform_type;

//extern "C" {
//    extern void glEGLImageTargetTexture2DOES (GLenum target, GLeglImageOES image);
//}

// ------------------------------------------------------------------------------------
// QSGVideoMaterialShader implementation
// ------------------------------------------------------------------------------------
QSGVideoMaterialShader::QSGVideoMaterialShader()
    : QSGMaterialShader()
{
}

char const *const *QSGVideoMaterialShader::attributeNames() const
{
	static const char *names[] = {
		attributeVertexPositionName(),
		attributeVertexTexCoordName(),
		"maskTexCoord",
		0
	};

	return names;
}

void QSGVideoMaterialShader::initialize()
{
	LOG_FUNC(">> Fn(QSGVideoMaterialShader::%s)\n", __func__);
	QSGMaterialShader::initialize();

	const char *uniformName = uniformMatrixName();
	if ( uniformName ) {
		m_id_matrix = program()->uniformLocation( uniformName );

		if ( m_id_matrix < 0 )
			qFatal( "Shader does not implement 'uniform %s' in its fragment shader \n", uniformName );
	} else {
		m_id_matrix = -1;
	}

	uniformName = uniformOpacityName();
	if ( uniformName ) {
		m_id_opacity = program()->uniformLocation( uniformName );

		if ( m_id_opacity < 0 )
			qFatal( "Shader does not implement 'uniform %s' in its fragment shader \n", uniformName );
	} else {
		m_id_opacity = -1;
	}

	for(int i = 0; i < ARGB_WINDOW_MAX; i++){
		uniformName = uniformExternalTexName(i);
		if ( uniformName ) {
			m_id_external_tex[i] = program()->uniformLocation( uniformName );
			LOG_PARAM("m_id_external_tex[%d] = %d\n", i, m_id_external_tex[i]);

			if ( m_id_external_tex[i] < 0 )
				LOG_SEQ( "Shader does not implement 'uniform %s' in its fragment shader \n", uniformName );
		} else {
			m_id_external_tex[i] = -1;
		}
	}

	uniformName = "mask";
	if ( uniformName ) {
		m_id_mask = program()->uniformLocation( uniformName );

		if ( m_id_mask < 0 ) {
			qFatal( "Shader does not implement 'uniform %s' in its fragment shader \n", uniformName );
		}
	} else {
		m_id_mask = -1;
	}
	LOG_FUNC("<< Fn(QSGVideoMaterialShader::%s)\n", __func__);
}
void QSGVideoMaterialShader::updateState( const RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial )
{
	LOG_FUNC(">> Fn(QSGVideoMaterialShader::%s)\n", __func__);
	Q_UNUSED( oldMaterial )
		QSGVideoMaterial *material = static_cast<QSGVideoMaterial *>( newMaterial );

	if ( state.isOpacityDirty() ) {
		material->setOpacity( state.opacity() );
		material->updateBlending();
		program()->setUniformValue( m_id_opacity, GLfloat( state.opacity() ) );
	}

	if ( state.isMatrixDirty() ) {
		program()->setUniformValue( m_id_matrix, state.combinedMatrix() );
	}

	QVideoImageStorage* qvideoImageStorageInstance = QVideoImageStorage::getInstance();
	if(qvideoImageStorageInstance->IsImageRegistartionDone(material->getWindowID())){

		//updateCounter();
		QOpenGLFunctions* gl = state.context()->functions();
		int windowID = material->getWindowID();
		program()->setUniformValue( m_id_external_tex[windowID], material->m_textureId);
		int texture_id = qvideoImageStorageInstance->renderNextFrame(material->getWindowID());
		material->m_textureId = texture_id;
		//qDebug()<<__FUNCTION__<<"extTextID"<<material->m_textureId;
		//program()->setUniformValue( m_id_mask, 1 );

	} else {
		LOG_FUNC("image register not ready for window %d\n", material->getWindowID() );
	}
	LOG_FUNC("<< Fn(QSGVideoMaterialShader::%s)\n", __func__);

}
const char * QSGVideoMaterialShader::vertexShader() const
{
	return
		"#ifdef GL_ES\n"
		"precision mediump int;\n"
		"precision mediump float;\n"
		"#endif\n"

		"attribute highp vec4 vertexPosition;\n"
		"uniform mediump mat4 matrix;\n"
		"attribute vec2 vertexTexCoord;\n"
		"attribute vec2 maskTexCoord;\n"
		"varying vec2 vtexCoord;\n"
		"varying vec2 vMaskTexCoord;\n"
		"void main(void)\n"
		"{\n"
		"    gl_Position = matrix * vertexPosition;\n"
		"    vtexCoord = vertexTexCoord;\n"
		"    vMaskTexCoord = maskTexCoord;\n"
		"}\n";
}

char shader_code[4096] = {0};
const char * QSGVideoMaterialShader::fragmentShader() const
{
	if ((platform_type == 0) || (platform_type == 1))
	{
#if (GRID_VAL == 55)
		// 320*180 with
		// 53 (in between) & 54 at the start & end - horizontal and
		// 30 vertical spacing
		snprintf(shader_code, 4096,
				"#extension GL_OES_EGL_image_external : require\n"
				"#ifdef GL_ES\n"
				"precision mediump int;\n"
				"precision mediump float;\n"
				"#endif\n"

				"%s"    //"uniform samplerExternalOES tex0;\n"
				"%s"    //"uniform samplerExternalOES tex1;\n"
				"%s"    //"uniform samplerExternalOES tex2;\n"
				"%s"    //"uniform samplerExternalOES tex3;\n"
				"%s"    //"uniform samplerExternalOES tex4;\n"
				"%s"    //"uniform samplerExternalOES tex5;\n"
				"%s"    //"uniform samplerExternalOES tex6;\n"
				"%s"    //"uniform samplerExternalOES tex7;\n"
				"%s"    //"uniform samplerExternalOES tex8;\n"
				"%s"    //"uniform samplerExternalOES tex9;\n"
				"%s"    //"uniform samplerExternalOES tex10;\n"
				"%s"    //"uniform samplerExternalOES tex11;\n"
				"%s"    //"uniform samplerExternalOES tex12;\n"
				"%s"    //"uniform samplerExternalOES tex13;\n"

				"uniform sampler2D mask;\n"
				"varying vec2 vtexCoord;\n"
				"varying vec2 vMaskTexCoord;\n"
				"uniform lowp float opacity;\n"
				"void main(void)\n"
				"{\n"
				"float m = 1. - texture2D(mask, vMaskTexCoord).r;\n"
				"if((%s) && gl_FragCoord.x >= 54.0 && gl_FragCoord.x < 374.0 \n"
				"&& gl_FragCoord.y >= 870.0 && gl_FragCoord.y < 1050.0 ) {\n"
				"%s"
				"}else if( (%s) && gl_FragCoord.x >= 427.0 && gl_FragCoord.x < 747.0 \n"
				"&& gl_FragCoord.y >= 870.0 && gl_FragCoord.y < 1050.0 ) {\n"
				"%s"
				"}else if( (%s) && gl_FragCoord.x >= 800.0 && gl_FragCoord.x < 1120.0\n"
				"&& gl_FragCoord.y >= 870.0 && gl_FragCoord.y < 1050.0 ) {\n"
				"%s"
				"}else if( (%s) && gl_FragCoord.x >= 1173.0 && gl_FragCoord.x < 1493.0\n"
				"&& gl_FragCoord.y >= 870.0 && gl_FragCoord.y < 1050.0 ) {\n"
				"%s"
				"}else if( (%s) && gl_FragCoord.x > 1546.0 && gl_FragCoord.x < 1866.0 \n"
				"&& gl_FragCoord.y >= 870.0 && gl_FragCoord.y < 1050.0 ) {\n"
				"%s"
				"}else if( (%s) && gl_FragCoord.x >= 54.0 && gl_FragCoord.x < 374.0 \n"
				"&& gl_FragCoord.y >= 660.0 && gl_FragCoord.y < 840.0 ) {\n"
				"%s"
				"}else if( (%s) && gl_FragCoord.x >= 427.0 && gl_FragCoord.x < 747.0 \n"
				"&& gl_FragCoord.y >= 660.0 && gl_FragCoord.y < 840.0 ) {\n"
				"%s"
				"}else if( (%s) && gl_FragCoord.x >= 54.0 && gl_FragCoord.x < 374.0 \n"
				"&& gl_FragCoord.y >= 450.0 && gl_FragCoord.y < 630.0 ) {\n"
				"%s"
				"}else if( (%s) && gl_FragCoord.x >= 427.0 && gl_FragCoord.x < 747.0\n"
				"&& gl_FragCoord.y >= 450.0 && gl_FragCoord.y < 630.0 ) {\n"
				"%s"
				"}else if( (%s) && gl_FragCoord.x >= 54.0 && gl_FragCoord.x < 374.0 \n"
				"&& gl_FragCoord.y >= 240.0 && gl_FragCoord.y < 420.0 ) {\n"
				"%s"
				"}else if( (%s) && gl_FragCoord.x >= 427.0 && gl_FragCoord.x < 747.0 \n"
				"&& gl_FragCoord.y >= 240.0 && gl_FragCoord.y < 420.0 ) {\n"
				"%s"
				"}else if( (%s) && gl_FragCoord.x >= 54.0 && gl_FragCoord.x < 374.0 \n"
				"&& gl_FragCoord.y >= 30.0 && gl_FragCoord.y < 210.0 ) {\n"
				"%s"
				"}else if( (%s) && gl_FragCoord.x >= 427.0 && gl_FragCoord.x < 747.0\n"
				"&& gl_FragCoord.y >= 30.0 && gl_FragCoord.y < 210.0 ) {\n"
				"%s"
				"}else if( (%s) && gl_FragCoord.x >= 54.0 && gl_FragCoord.x < 1866.0\n"
				"&& gl_FragCoord.y >= 30.0 && gl_FragCoord.y < 1050.0 ) {\n"
				"%s"
				"}else{\n"
				"gl_FragColor = vec4(0.0,0.0,1.0,1.0);\n"
				"}\n"
				"}\n",
			fVisible[0] ? "uniform samplerExternalOES tex0;\n" : "",
			fVisible[1] ? "uniform samplerExternalOES tex1;\n" : "",
			fVisible[2] ? "uniform samplerExternalOES tex2;\n" : "",
			fVisible[3] ? "uniform samplerExternalOES tex3;\n" : "",
			fVisible[4] ? "uniform samplerExternalOES tex4;\n" : "",
			fVisible[5] ? "uniform samplerExternalOES tex5;\n" : "",
			fVisible[6] ? "uniform samplerExternalOES tex6;\n" : "",
			fVisible[7] ? "uniform samplerExternalOES tex7;\n" : "",
			fVisible[8] ? "uniform samplerExternalOES tex8;\n" : "",
			fVisible[9] ? "uniform samplerExternalOES tex9;\n" : "",
			fVisible[10] ? "uniform samplerExternalOES tex10;\n" : "",
			fVisible[11] ? "uniform samplerExternalOES tex11;\n" : "",
			fVisible[12] ? "uniform samplerExternalOES tex12;\n" : "",
			fVisible[13] ? "uniform samplerExternalOES tex13;\n" : "",
			fVisible[0] ? "true" : "false",
			fVisible[0] ? "gl_FragColor = texture2D(tex0, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[1] ? "true" : "false",
			fVisible[1] ? "gl_FragColor = texture2D(tex1, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[2] ? "true" : "false",
			fVisible[2] ? "gl_FragColor = texture2D(tex2, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[3] ? "true" : "false",
			fVisible[3] ? "gl_FragColor = texture2D(tex3, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[4] ? "true" : "false",
			fVisible[4] ? "gl_FragColor = texture2D(tex4, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[5] ? "true" : "false",
			fVisible[5] ? "gl_FragColor = texture2D(tex5, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[6] ? "true" : "false",
			fVisible[6] ? "gl_FragColor = texture2D(tex6, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[7] ? "true" : "false",
			fVisible[7] ? "gl_FragColor = texture2D(tex7, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[8] ? "true" : "false",
			fVisible[8] ? "gl_FragColor = texture2D(tex8, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[9] ? "true" : "false",
			fVisible[9] ? "gl_FragColor = texture2D(tex9, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[10] ? "true" : "false",
			fVisible[10] ? "gl_FragColor = texture2D(tex10, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[11] ? "true" : "false",
			fVisible[11] ? "gl_FragColor = texture2D(tex11, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[12] ? "true" : "false",
			fVisible[12] ? "gl_FragColor = texture2D(tex12, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[13] ? "true" : "false",
			fVisible[13] ? "gl_FragColor = texture2D(tex13, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n"
			);
#else
		// 445*250 with 28 - horizontal and 16 vertical spacing
		snprintf(shader_code, 4096,
				"#extension GL_OES_EGL_image_external : require\n"
				"#ifdef GL_ES\n"
				"precision mediump int;\n"
				"precision mediump float;\n"
				"#endif\n"

				"%s"    //"uniform samplerExternalOES tex0;\n"
				"%s"    //"uniform samplerExternalOES tex1;\n"
				"%s"    //"uniform samplerExternalOES tex2;\n"
				"%s"    //"uniform samplerExternalOES tex3;\n"
				"%s"    //"uniform samplerExternalOES tex4;\n"
				"%s"    //"uniform samplerExternalOES tex5;\n"
				"%s"    //"uniform samplerExternalOES tex6;\n"
				"%s"    //"uniform samplerExternalOES tex7;\n"
				"%s"    //"uniform samplerExternalOES tex8;\n"
				"%s"    //"uniform samplerExternalOES tex9;\n"
				"%s"    //"uniform samplerExternalOES tex10;\n"
				"%s"    //"uniform samplerExternalOES tex11;\n"
				"%s"    //"uniform samplerExternalOES tex12;\n"
				"%s"    //"uniform samplerExternalOES tex13;\n"
				"%s"    //"uniform samplerExternalOES tex14;\n"
				"%s"    //"uniform samplerExternalOES tex15;\n"

				"uniform sampler2D mask;\n"
				"varying highp vec2 vtexCoord;\n"
				"varying highp vec2 vMaskTexCoord;\n"
				"uniform lowp float opacity;\n"
				"void main(void)\n"
				"{\n"
				"float m = 1. - texture2D(mask, vMaskTexCoord).r;\n"
				"if( gl_FragCoord.x >= 28.0 && gl_FragCoord.x < 473.0 \n"
				"&& gl_FragCoord.y >= 814.0 && gl_FragCoord.y < 1064.0 ) {\n"
				"%s"
				"}else if( gl_FragCoord.x >= 501.0 && gl_FragCoord.x < 946.0 \n"
				"&& gl_FragCoord.y >= 814.0 && gl_FragCoord.y < 1064.0 ) {\n"
				"%s"
				"}else if( gl_FragCoord.x >= 974.0 && gl_FragCoord.x < 1419.0\n"
				"&& gl_FragCoord.y >= 814.0 && gl_FragCoord.y < 1064.0 ) {\n"
				"%s"
				"}else if( gl_FragCoord.x >= 1447.0 && gl_FragCoord.x < 1892.0\n"
				"&& gl_FragCoord.y >= 814.0 && gl_FragCoord.y < 1064.0 ) {\n"
				"%s"
				"}else if( gl_FragCoord.x > 28.0 && gl_FragCoord.x < 473.0 \n"
				"&& gl_FragCoord.y >= 548.0 && gl_FragCoord.y < 798.0 ) {\n"
				"%s"
				"}else if( gl_FragCoord.x >= 501.0 && gl_FragCoord.x < 946.0 \n"
				"&& gl_FragCoord.y >= 548.0 && gl_FragCoord.y < 798.0 ) {\n"
				"%s"
				"}else if( gl_FragCoord.x >= 974.0 && gl_FragCoord.x < 1419.0 \n"
				"&& gl_FragCoord.y >= 548.0 && gl_FragCoord.y < 798.0 ) {\n"
				"%s"
				"}else if( gl_FragCoord.x >= 1447.0 && gl_FragCoord.x < 1892.0\n"
				"&& gl_FragCoord.y >= 548.0 && gl_FragCoord.y < 798.0 ) {\n"
				"%s"
				"}else if( gl_FragCoord.x > 28.0 && gl_FragCoord.x < 473.0 \n"
				"&& gl_FragCoord.y >= 282.0 && gl_FragCoord.y < 532.0 ) {\n"
				"%s"
				"}else if( gl_FragCoord.x >= 501.0 && gl_FragCoord.x < 946.0 \n"
				"&& gl_FragCoord.y >= 282.0 && gl_FragCoord.y < 532.0 ) {\n"
				"%s"
				"}else if( gl_FragCoord.x >= 974.0 && gl_FragCoord.x < 1419.0 \n"
				"&& gl_FragCoord.y >= 282.0 && gl_FragCoord.y < 532.0 ) {\n"
				"%s"
				"}else if( gl_FragCoord.x >= 1447.0 && gl_FragCoord.x < 1892.0\n"
				"&& gl_FragCoord.y >= 282.0 && gl_FragCoord.y < 532.0 ) {\n"
				"%s"
				"}else if( gl_FragCoord.x > 28.0 && gl_FragCoord.x < 473.0 \n"
				"&& gl_FragCoord.y >= 16.0 && gl_FragCoord.y < 266.0 ) {\n"
				"%s"
				"}else if( gl_FragCoord.x >= 501.0 && gl_FragCoord.x < 946.0 \n"
				"&& gl_FragCoord.y >= 16.0 && gl_FragCoord.y < 266.0 ) {\n"
				"%s"
				"}else if( gl_FragCoord.x >= 974.0 && gl_FragCoord.x < 1419.0 \n"
				"&& gl_FragCoord.y >= 16.0 && gl_FragCoord.y < 266.0 ) {\n"
				"%s"
				"}else if( gl_FragCoord.x >= 1447.0 && gl_FragCoord.x < 1892.0\n"
				"&& gl_FragCoord.y >= 16.0 && gl_FragCoord.y < 266.0 ) {\n"
				"%s"
				"}else{\n"
				"gl_FragColor = vec4(0.0,0.0,1.0,1.0);\n"
				"}\n"
				"}\n",
			fVisible[0] ? "uniform samplerExternalOES tex0;\n" : "",
			fVisible[1] ? "uniform samplerExternalOES tex1;\n" : "",
			fVisible[2] ? "uniform samplerExternalOES tex2;\n" : "",
			fVisible[3] ? "uniform samplerExternalOES tex3;\n" : "",
			fVisible[4] ? "uniform samplerExternalOES tex4;\n" : "",
			fVisible[5] ? "uniform samplerExternalOES tex5;\n" : "",
			fVisible[6] ? "uniform samplerExternalOES tex6;\n" : "",
			fVisible[7] ? "uniform samplerExternalOES tex7;\n" : "",
			fVisible[8] ? "uniform samplerExternalOES tex8;\n" : "",
			fVisible[9] ? "uniform samplerExternalOES tex9;\n" : "",
			fVisible[10] ? "uniform samplerExternalOES tex10;\n" : "",
			fVisible[11] ? "uniform samplerExternalOES tex11;\n" : "",
			fVisible[12] ? "uniform samplerExternalOES tex12;\n" : "",
			fVisible[13] ? "uniform samplerExternalOES tex13;\n" : "",
			fVisible[14] ? "uniform samplerExternalOES tex14;\n" : "",
			fVisible[15] ? "uniform samplerExternalOES tex15;\n" : "",
			fVisible[0] ? "gl_FragColor = texture2D(tex0, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[1] ? "gl_FragColor = texture2D(tex1, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[2] ? "gl_FragColor = texture2D(tex2, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[3] ? "gl_FragColor = texture2D(tex3, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[4] ? "gl_FragColor = texture2D(tex4, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[5] ? "gl_FragColor = texture2D(tex5, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[6] ? "gl_FragColor = texture2D(tex6, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[7] ? "gl_FragColor = texture2D(tex7, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[8] ? "gl_FragColor = texture2D(tex8, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[9] ? "gl_FragColor = texture2D(tex9, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[10] ? "gl_FragColor = texture2D(tex10, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[11] ? "gl_FragColor = texture2D(tex11, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[12] ? "gl_FragColor = texture2D(tex12, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[13] ? "gl_FragColor = texture2D(tex13, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[14] ? "gl_FragColor = texture2D(tex14, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[15] ? "gl_FragColor = texture2D(tex15, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n"
				);
#endif
	}
	else
	{

		// 800*450 with
		// 0 horizontal indexing and
		// 130 vertical spacing at the beginning and end, with 120 between 2 units
		snprintf(shader_code, 4096,
				"#extension GL_OES_EGL_image_external : require\n"
				"#ifdef GL_ES\n"
				"precision mediump int;\n"
				"precision mediump float;\n"
				"#endif\n"

				"%s"    //"uniform samplerExternalOES tex0;\n"
				"%s"    //"uniform samplerExternalOES tex1;\n"

				"uniform sampler2D mask;\n"
				"varying vec2 vtexCoord;\n"
				"varying vec2 vMaskTexCoord;\n"
				"uniform lowp float opacity;\n"
				"void main(void)\n"
				"{\n"
				"float m = 1. - texture2D(mask, vMaskTexCoord).r;\n"
				"if( gl_FragCoord.x >= 0.0 && gl_FragCoord.x < 800.0 \n"
				"&& gl_FragCoord.y >= 710.0 && gl_FragCoord.y < 1160.0 ) {\n"
				"%s"
				"}else if( gl_FragCoord.x >= 0.0 && gl_FragCoord.x < 800.0 \n"
				"&& gl_FragCoord.y >= 120.0 && gl_FragCoord.y < 670.0 ) {\n"
				"%s"
				"}else{\n"
				"gl_FragColor = vec4(0.0,0.0,1.0,1.0);\n"
				"}\n"
				"}\n",
			fVisible[0] ? "uniform samplerExternalOES tex0;\n" : "",
			fVisible[1] ? "uniform samplerExternalOES tex1;\n" : "",
			fVisible[0] ? "gl_FragColor = texture2D(tex0, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n",
			fVisible[1] ? "gl_FragColor = texture2D(tex1, vtexCoord) * (opacity * m);\n" : "gl_FragColor = vec4(0.0,0.0,0.0,0.0);\n"
				);
	}
	LOG_PARAM("%s \n", shader_code);
	return shader_code;
}

const char * QSGVideoMaterialShader::uniformMatrixName() const
{
	return "matrix";
}

const char * QSGVideoMaterialShader::uniformOpacityName() const
{
	return "opacity";
}

const char *pTexName[ARGB_WINDOW_MAX] = { "tex0",
	"tex1",
	"tex2",
	"tex3",
	"tex4",
	"tex5",
	"tex6",
	"tex7",
	"tex8",
	"tex9",
	"tex10",
	"tex11",
	"tex12",
	"tex13",
	"tex14",
	"tex15"
};

const char *QSGVideoMaterialShader::uniformExternalTexName(int i) const
{
	const char *pTex;
	if(i < ARGB_WINDOW_MAX)
		pTex = pTexName[i];
	else
		pTex = NULL;

	return pTex;
}

const char * QSGVideoMaterialShader::attributeVertexPositionName() const
{
	return "vertexPosition";
}

const char * QSGVideoMaterialShader::attributeVertexTexCoordName() const
{
	return "vertexTexCoord";
}

//QList<VIDEO_DATA_REF> QSGVideoMaterial::reserved_preivew_refList = QList<VIDEO_DATA_REF>();
// ------------------------------------------------------------------------------------
// rendering state
// ------------------------------------------------------------------------------------
QSGVideoMaterial::QSGVideoMaterial(int windowID)
    : QSGMaterial()
    , m_textureId( 0 )
    , m_eglImage( EGL_NO_IMAGE_KHR )
    , m_opacity( 1.0 )
    , m_maskTextureProvider( NULL )
    , m_windowID(windowID)
{
	setFlag( Blending, false );
}

QSGVideoMaterial::QSGVideoMaterial(VideoItem *item)
    : QSGMaterial()
    , m_textureId( 0 )
    , m_eglImage( EGL_NO_IMAGE_KHR )
    , m_opacity( 1.0 )
    , m_maskTextureProvider( NULL )
    , m_windowID(item->getWindowID())
{
	m_item = item;
	setFlag( Blending, false );
}

QSGVideoMaterial::~QSGVideoMaterial()
{
	//    disposeEglImage();
}

//void QSGVideoMaterial::disposeEglImage()
//{
//    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
//    glDeleteTextures(1, &m_textureId);
//    EGLDisplay display = eglGetCurrentDisplay();
//    eglDestroyImageKHR( display, m_eglImage );
//    m_eglImage = 0; //EGL_NO_IMAGE_KHR;
//}

void QSGVideoMaterial::initialize()
{
	LOG_FUNC(">> Fn(QSGVideoMaterial::%s)\n", __func__);
	QVideoImageStorage::getInstance()->updateEGLDisplay((int)m_windowID);
	LOG_FUNC("<< Fn(QSGVideoMaterial::%s)\n", __func__);
}

QSGMaterialType *QSGVideoMaterial::type() const
{
	static QSGMaterialType theType;
	return &theType;
}

QSGMaterialShader *QSGVideoMaterial::createShader() const
{
	LOG_FUNC(">> Fn(QSGVideoMaterial::%s)\n", __func__);
	QSGMaterialShader *shader = new QSGVideoMaterialShader();
	LOG_FUNC("<< Fn(QSGVideoMaterial::%s)\n", __func__);
	return shader;
}

int QSGVideoMaterial::compare( const QSGMaterial *other ) const
{
	LOG_FUNC(">> Fn(QSGVideoMaterial::%s)\n", __func__);
	int ret;
	const QSGVideoMaterial *m = static_cast<const QSGVideoMaterial *>( other );

	int diff = m_textureId - m->m_textureId;
	if ( diff ){
		ret = diff;
	}else{
		ret = ( m_opacity > m->m_opacity ) ? 1 : -1;
	}
	LOG_FUNC("<< Fn(QSGVideoMaterial::%s)\n", __func__);
	return ret;
}

void QSGVideoMaterial::updateBlending()
{
	LOG_FUNC(">> Fn(QSGVideoMaterial::%s)\n", __func__);
	setFlag( Blending, (qFuzzyCompare( m_opacity, qreal( 1.0 ) ) && !m_maskTextureProvider) ? false : true );
	LOG_FUNC("<< Fn(QSGVideoMaterial::%s)\n", __func__);
}

void QSGVideoMaterial::setOpacity( qreal opacity )
{
	m_opacity = opacity;
}

void QSGVideoMaterial::setMaskTextureProvider(QSGTextureProvider* tp) {
	m_maskTextureProvider = tp;
	updateBlending();
}

void QSGVideoMaterial::bind()
{
	LOG_FUNC(">> Fn(QSGVideoMaterial::%s)\n", __func__);
	glBindTexture( GL_TEXTURE_EXTERNAL_OES, m_textureId );
	LOG_FUNC("<< Fn(QSGVideoMaterial::%s)\n", __func__);
}

void QSGVideoMaterial::bindMaskTexture()
{
	LOG_FUNC(">> Fn(QSGVideoMaterial::%s)\n", __func__);
	if (m_maskTextureProvider) {
		QSGTexture* tex = m_maskTextureProvider->texture();
		if (tex) {
			if (tex->isAtlasTexture()) {
				tex = tex->removedFromAtlas();
			}
			tex->bind();
			return;
		}
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	LOG_FUNC("<< Fn(QSGVideoMaterial::%s)\n", __func__);
}

void QSGVideoMaterial::setWindowID(int windowID)
{
	LOG_FUNC(">> Fn(QSGVideoMaterial::%s)\n", __func__);
	m_windowID = windowID;
	LOG_FUNC("<< Fn(QSGVideoMaterial::%s)\n", __func__);
}

void QSGVideoMaterial::update(VideoStream* stream)
{
	LOG_FUNC(">> Fn(QSGVideoMaterial::%s)\n", __func__);
	Q_UNUSED(stream);
	LOG_FUNC("<< Fn(QSGVideoMaterial::%s)\n", __func__);
}

void QSGVideoMaterial::update()
{
	LOG_FUNC(">> Fn(QSGVideoMaterial::%s)\n", __func__);
	VideoStream* stream = VideoItemUpdater::getVideoStream(m_windowID);
	// static item may come here too early
	//    if (stream && stream->getFrame()._videoDataRef)
	{
		stream->lock();
		update(stream);
		stream->unlock();
	}
	LOG_FUNC("<< Fn(QSGVideoMaterial::%s)\n", __func__);
}


// ------------------------------------------------------------------------------------
//geometry and material
// ------------------------------------------------------------------------------------
struct TexturedMaskedPoint2D {
	float x, y;
	float tx, ty; // coordinates in the video texture
	float mx, my; // coordinates in the mask texture
	void set(float nx, float ny, float ntx, float nty, float nmx, float nmy) {
		x = nx; y = ny; tx = ntx; ty = nty; mx = nmx; my = nmy;
	}
};

QSGVideoNode::QSGVideoNode(VideoItem* videoItem)
    : QSGGeometryNode()
{
	LOG_FUNC(">> Fn(QSGVideoNode::%s)\n", __func__);
	m_videoItem = videoItem;
	m_maskTextureProvider = NULL;
	m_material = new QSGVideoMaterial(videoItem->getWindowID());
	m_textureVec = NULL;

	setMaterial( m_material );
	setFlag( QSGNode::OwnsMaterial );
	LOG_FUNC("<< Fn(QSGVideoNode::%s)\n", __func__);
}

QSGVideoNode::~QSGVideoNode()
{
}

void QSGVideoNode::setRectGeometry( const QRectF &boundingRect, const QRectF &textureRect, float maskBorder)
{
	LOG_FUNC(">> Fn(QSGVideoNode::%s)\n", __func__);
	if ( ( boundingRect == m_rect ) && ( textureRect == m_textureRect ) )
		return;

	m_rect = boundingRect;
	m_textureRect = textureRect;

	QSGGeometry *g = geometry();

	if ( g == 0 ) {
		static QSGGeometry::Attribute data[] = {
			QSGGeometry::Attribute::create(0, 2, GL_FLOAT, true),
			QSGGeometry::Attribute::create(1, 2, GL_FLOAT),
			QSGGeometry::Attribute::create(2, 2, GL_FLOAT)
		};
		static QSGGeometry::AttributeSet attrs = { 3, sizeof(float) * 6, data };
		// 9 patches * 2 triangles * 3 verticies
		g = new QSGGeometry( attrs, 16, 3*2*9 );

		setFlag ( QSGNode::OwnsGeometry );

		g->setDrawingMode(/*GL_TRIANGLES*/ 0x0004);
		quint16* indexes = g->indexDataAsUShort();
		/*
		 * Nine patch mesh layout:
		 *  0--4----8-12
		 *  | /|  / | /|
		 *  1--5----9-13
		 *  | /|  / | /|
		 *  |/ | /  |/ |
		 *  2--6---10-14
		 *  | /|  / | /|
		 *  3--7---11-15
		 */
		// Top left
		indexes[0] = 0;
		indexes[1] = 4;
		indexes[2] = 1;
		indexes[3] = 1;
		indexes[4] = 4;
		indexes[5] = 5;

		// Center left
		indexes[6] = 1;
		indexes[7] = 5;
		indexes[8] = 2;
		indexes[9] = 2;
		indexes[10] = 5;
		indexes[11] = 6;

		// Bottom left
		indexes[12] = 2;
		indexes[13] = 6;
		indexes[14] = 3;
		indexes[15] = 3;
		indexes[16] = 6;
		indexes[17] = 7;

		// Top center
		indexes[18] = 4;
		indexes[19] = 8;
		indexes[20] = 5;
		indexes[21] = 5;
		indexes[22] = 8;
		indexes[23] = 9;

		// Center
		indexes[24] = 5;
		indexes[25] = 9;
		indexes[26] = 6;
		indexes[27] = 6;
		indexes[28] = 9;
		indexes[29] = 10;

		// Bottom center
		indexes[30] = 6;
		indexes[31] = 10;
		indexes[32] = 7;
		indexes[33] = 7;
		indexes[34] = 10;
		indexes[35] = 11;

		// Top right
		indexes[36] = 8;
		indexes[37] = 12;
		indexes[38] = 9;
		indexes[39] = 9;
		indexes[40] = 12;
		indexes[41] = 13;

		// Center right
		indexes[42] = 9;
		indexes[43] = 13;
		indexes[44] = 10;
		indexes[45] = 10;
		indexes[46] = 13;
		indexes[47] = 14;

		// Bottom right
		indexes[48] = 10;
		indexes[49] = 14;
		indexes[50] = 11;
		indexes[51] = 11;
		indexes[52] = 14;
		indexes[53] = 15;
	}

	m_textureVec = static_cast<TexturedMaskedPoint2D*>(g->vertexData());

	// Set geometry first
	QPointF gPt, txPt;
	float b = maskBorder;
	float bx = b*m_textureRect.width()/m_rect.width();
	float by = b*m_textureRect.height()/m_rect.height();

	gPt = m_rect.topLeft();
	txPt = m_textureRect.topLeft();
	m_textureVec[0].set( gPt.x(), gPt.y(), txPt.x(), txPt.y(), 0.0f, 0.0f );
	m_textureVec[1].set( gPt.x(), gPt.y()+b, txPt.x(), txPt.y()+by, 0.0f, 0.5f );
	m_textureVec[4].set( gPt.x()+b, gPt.y(), txPt.x()+bx, txPt.y(), 0.5f, 0.0f );
	m_textureVec[5].set( gPt.x()+b, gPt.y()+b, txPt.x()+bx, txPt.y()+by, 0.5f, 0.5f );

	gPt = m_rect.bottomLeft();
	txPt = m_textureRect.bottomLeft();
	m_textureVec[2].set( gPt.x(), gPt.y()-b, txPt.x(), txPt.y()-by, 0.0f, 0.5f );
	m_textureVec[3].set( gPt.x(), gPt.y(), txPt.x(), txPt.y(), 0.0f, 1.0f );
	m_textureVec[6].set( gPt.x()+b, gPt.y()-b, txPt.x()+bx, txPt.y()-by, 0.5f, 0.5f );
	m_textureVec[7].set( gPt.x()+b, gPt.y(), txPt.x()+bx, txPt.y(), 0.5f, 1.0f );

	gPt = m_rect.topRight();
	txPt = m_textureRect.topRight();
	m_textureVec[8].set( gPt.x()-b, gPt.y(), txPt.x()-bx, txPt.y(), 0.5f, 0.0f );
	m_textureVec[9].set( gPt.x()-b, gPt.y()+b, txPt.x()-bx, txPt.y()+by, 0.5f, 0.5f );
	m_textureVec[12].set( gPt.x(), gPt.y(), txPt.x(), txPt.y(), 1.0f, 0.0f );
	m_textureVec[13].set( gPt.x(), gPt.y()+b, txPt.x(), txPt.y()+by, 1.0f, 0.5f );

	gPt = m_rect.bottomRight();
	txPt = m_textureRect.bottomRight();
	m_textureVec[10].set( gPt.x()-b, gPt.y()-b, txPt.x()-bx, txPt.y()-by, 0.5f, 0.5f );
	m_textureVec[11].set( gPt.x()-b, gPt.y(), txPt.x()-bx, txPt.y(), 0.5f, 1.0f );
	m_textureVec[14].set( gPt.x(), gPt.y()-b, txPt.x(), txPt.y()-by, 1.0f, 0.5f );
	m_textureVec[15].set( gPt.x(), gPt.y(), txPt.x(), txPt.y(), 1.0f, 1.0f );

	if ( ! geometry() )
		setGeometry( g );

	markDirty( DirtyGeometry );
	LOG_FUNC("<< Fn(QSGVideoNode::%s)\n", __func__);
}

void QSGVideoNode::setMaskTextureProvider(QSGTextureProvider* tp) {
	LOG_FUNC(">> Fn(QSGVideoNode::%s)\n", __func__);
	if (m_maskTextureProvider == tp) {
		return;
	}
	if (m_maskTextureProvider) {
		QObject::disconnect(m_maskTextureProvider, SIGNAL(destroyed(QObject*)), this, SLOT(maskTextureProviderDestroyed(QObject*)));
	}
	m_maskTextureProvider = tp;
	if (m_maskTextureProvider) {
		QObject::connect(m_maskTextureProvider, SIGNAL(destroyed(QObject*)), this, SLOT(maskTextureProviderDestroyed(QObject*)));
	}

	m_material->setMaskTextureProvider(m_maskTextureProvider);

	markDirty( DirtyMaterial );
	LOG_FUNC("<< Fn(QSGVideoNode::%s)\n", __func__);
}

void QSGVideoNode::maskTextureProviderDestroyed(QObject *object) {
	Q_UNUSED(object);
	m_maskTextureProvider = 0;
	m_material->setMaskTextureProvider(m_maskTextureProvider);
	markDirty( DirtyMaterial );
}

// ------------------------------------------------------------------------------------
//QML item
// ------------------------------------------------------------------------------------
VideoItem::VideoItem( QQuickItem *parent )
//    : QQuickItem(parent)
    : QQuickVideoImage( parent )
    , m_windowID( INVALID_VIDEO_TYPE )
    , m_maskBorder( 0.f )
    , m_textureRect( QRectF(0, 0, 1, 1) )
    , m_maskImage( NULL )
    , m_material( NULL )
    , m_videoStream(NULL)
{
	LOG_FUNC(">> Fn(VideoItem::%s)\n", __func__);
	connect( this, &VideoItem::frameDataChanged, this, &VideoItem::update, Qt::QueuedConnection );
	// Important, otherwise the paint method is never called
	setFlag( ItemHasContents, true );

	// QVideoImageStorage::getInstance()->updateEGLDisplay();
	connect(this, SIGNAL(heightChanged()),this, SLOT(videoImageUpdated()));
	connect(this, SIGNAL(widthChanged()), this, SLOT(videoImageUpdated()));
	LOG_FUNC("<< Fn(VideoItem::%s)\n", __func__);
}

VideoItem::~VideoItem()
{
	if (m_videoStream) {
		m_videoStream->clearVideoItemIfEqual(this);
		m_videoStream = NULL;
	}
}

void VideoItem::updateFrame()
{
	LOG_FUNC(">> Fn(VideoItem::%s)\n", __func__);
	emit frameDataChanged();
	LOG_FUNC("<< Fn(VideoItem::%s)\n", __func__);
}

void VideoItem::setWindowID( const int windowID )
{
	LOG_FUNC(">> Fn(VideoItem::%s)\n", __func__);
	if ( m_windowID != windowID ) {
		m_windowID = windowID;
		QQuickVideoImage::m_windowID = (ARGBWindowID)windowID;

		updateVideoStream();

		if (m_material)
			m_material->setWindowID(windowID);

		emit videoTypeChanged();
	}
	LOG_FUNC("<< Fn(VideoItem::%s)\n", __func__);
}

void VideoItem::setResolutionWidth( const int resolutionWidth )
{
	LOG_FUNC(">> Fn(VideoItem::%s)\n", __func__);
	if ( m_resolutionWidth != resolutionWidth ) {
		m_resolutionWidth = resolutionWidth;
		emit resolutionWidthChanged();
	}
	LOG_FUNC("<< Fn(VideoItem::%s)\n", __func__);
}

void VideoItem::setResolutionHeight( const int resolutionHeight )
{
	LOG_FUNC(">> Fn(VideoItem::%s)\n", __func__);
	if ( m_resolutionHeight != resolutionHeight ) {
		m_resolutionHeight = resolutionHeight;
		emit resolutionHeightChanged();
	}
	LOG_FUNC("<< Fn(VideoItem::%s)\n", __func__);
}

void VideoItem::setMaskImage( QQuickItem* maskImage ) {
	m_maskImage = maskImage;
}

float VideoItem::maskBorder() const {
	return m_maskBorder;
}

void VideoItem::setMaskBorder( float border ) {
	m_maskBorder = border;
}

QSGNode* VideoItem::updatePaintNode( QSGNode *node, UpdatePaintNodeData *)
{
	LOG_FUNC(">> Fn(VideoItem::%s)\n", __func__);
	QSGVideoNode *videoNode = static_cast<QSGVideoNode *>( node );

	if ( ! videoNode ) {
		videoNode = new QSGVideoNode(this);

		static_cast<QSGVideoMaterial *>( videoNode->material() )->initialize();
	}

	m_material = static_cast<QSGVideoMaterial *>( videoNode->material() );
	m_material->update();

	videoNode->markDirty( QSGNode::DirtyMaterial );

	videoNode->setRectGeometry( boundingRect(), m_textureRect, m_maskBorder );

	videoNode->markDirty( QSGNode::DirtyGeometry );

	if (m_maskImage && m_maskImage->isTextureProvider()) {
		videoNode->setMaskTextureProvider( m_maskImage->textureProvider() );
	} else {
		videoNode->setMaskTextureProvider( 0 );
	}
	LOG_FUNC("<< Fn(VideoItem::%s)\n", __func__);

	return videoNode;
}

void VideoItem::itemChange(ItemChange change, const ItemChangeData & value)
{
	LOG_FUNC(">> Fn(VideoItem::%s)\n", __func__);
	if (change == QQuickItem::ItemVisibleHasChanged) {
		if (value.boolValue)
			updateVideoStream();
	}
	LOG_FUNC("<< Fn(VideoItem::%s)\n", __func__);
}

void VideoItem::updateVideoStream()
{
	LOG_FUNC(">> Fn(VideoItem::%s)\n", __func__);
	VideoStream* newVideoStream = VideoItemUpdater::getVideoStream(m_windowID);
	if (!newVideoStream)
		return;

	if (newVideoStream != m_videoStream) {
		m_videoStream = newVideoStream;
	}

	m_videoStream->setVideoItem(this);
	LOG_FUNC("<< Fn(VideoItem::%s)\n", __func__);
}

#include "qvideoimage.h"
#include "qvideoimagestorage.h"
void VideoItem::videoImageUpdated()
{
	LOG_FUNC(">> Fn(VideoItem::%s)\n", __func__);
	if(width() && height()){
		//Image is ready for registrattion
		LOG_PARAM("###############The value of width %f and height %f\n",width(),height());
		if(m_windowID < ARGB_WINDOW_MAX && m_windowID >= 0)
			QVideoImageStorage::getInstance()->registerQVideoImage(this, (ARGBWindowID) m_windowID,&QQuickVideoImage::processMessage);
	}
	LOG_FUNC("<< Fn(VideoItem::%s)\n", __func__);
}
