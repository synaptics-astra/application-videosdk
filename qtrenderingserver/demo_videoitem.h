/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2024 Synaptics Incorporated */

#ifndef DEMO_VIDEOITEM_H
#define DEMO_VIDEOITEM_H

#include "demo_videostream.h"

#include <QtCore/QMutex>
#include <QtQuick/qsgnode.h>
#include <QtQuick/qsgmaterial.h>
#include <QtQuick/QSGMaterialShader>
#include <QQuickItem>

#ifdef DEMO_COMPILE
#include "demo_mock.h"
#else
#include <EGL/egl.h>
#include "EGL/eglext.h"
#endif


class QSGTextureProvider;

//------------------OpenGL shader program---------------------------
class QSGVideoMaterialShader : public QSGMaterialShader
{
public:
    QSGVideoMaterialShader();

    virtual char const *const *attributeNames() const;

    virtual void updateState( const RenderState &state, QSGMaterial *newMaterial, QSGMaterial *oldMaterial );

protected:
    virtual void initialize();

    virtual const char *vertexShader() const;
    virtual const char *fragmentShader() const;

    const char * uniformMatrixName() const;
    const char * uniformOpacityName() const;
    const char * uniformExternalTexName(int) const;

    const char * attributeVertexPositionName() const;
    const char * attributeVertexTexCoordName() const;

private:
    int m_id_matrix;
    int m_id_opacity;
    int m_id_external_tex[ARGB_WINDOW_MAX];
    int m_id_mask;
};

class TexturedMaskedPoint2D;
class VideoItem;
// --------------------------------rendering state-----------------------------------
class QSGVideoMaterial : public QSGMaterial
{
public:
    QSGVideoMaterial(int windowID);
    QSGVideoMaterial(VideoItem* item);
    ~QSGVideoMaterial();

    virtual void initialize();

    virtual QSGMaterialType *type() const;
    virtual QSGMaterialShader *createShader() const;
    virtual int compare( const QSGMaterial *other ) const;

    void updateBlending();
    void setOpacity( qreal opacity );
    void bind();
    void setMaskTextureProvider(QSGTextureProvider* tp);
    void bindMaskTexture();
    int getWindowID() const { return m_windowID; }
    void setWindowID(int windowID);
    void update();

    GLuint m_textureId;

protected:
    void update(VideoStream*);
    void* m_eglImage;
    qreal m_opacity;

    QSGTextureProvider* m_maskTextureProvider;
    int m_windowID;
    VideoItem* m_item;
};

//------------------geometry and material---------------------------
class QSGVideoNode : public QObject, public QSGGeometryNode
{
    Q_OBJECT
public:
    QSGVideoNode(VideoItem*);
    virtual ~QSGVideoNode();

    void setRectGeometry( const QRectF &boundingRect, const QRectF &textureRect, float maskBorder );
    void setMaskTextureProvider(QSGTextureProvider* tp);

protected:
    QSGVideoMaterial *m_material;
    QRectF m_rect;
    QRectF m_textureRect;
    QSGTextureProvider* m_maskTextureProvider;

private Q_SLOTS:
    void maskTextureProviderDestroyed(QObject *object);

private:
    VideoItem* m_videoItem;
    TexturedMaskedPoint2D* m_textureVec;
};

#include "qvideoimage.h"
//------------------QML item---------------------------
class VideoItem : public QQuickVideoImage /*QQuickItem*/
{
    Q_OBJECT
    Q_PROPERTY( int windowID READ getWindowID WRITE setWindowID NOTIFY videoTypeChanged )
    Q_PROPERTY( int resolutionWidth READ resolutionWidth WRITE setResolutionWidth NOTIFY resolutionWidthChanged )
    Q_PROPERTY( int resolutionHeight READ resolutionHeight WRITE setResolutionHeight NOTIFY resolutionHeightChanged )
    Q_PROPERTY( QQuickItem* maskImage READ maskImage WRITE setMaskImage NOTIFY maskImageChanged )
    Q_PROPERTY( float maskBorder READ maskBorder WRITE setMaskBorder NOTIFY maskBorderChanged )

public:
    VideoItem( QQuickItem *parent = 0 );
    ~VideoItem();

    void updateFrame();
    void setWindowID( const int windowID );
    void setResolutionWidth( const int width );
    void setResolutionHeight( const int height );

    int getWindowID() const { return m_windowID; }
    int resolutionWidth() const { return m_resolutionWidth; }
    int resolutionHeight() const { return m_resolutionHeight; }
    QQuickItem* maskImage() const { return m_maskImage; }

    void setMaskImage( QQuickItem* maskImage );
    float maskBorder() const;
    void setMaskBorder( float border );

    QSGNode* updatePaintNode( QSGNode *node, UpdatePaintNodeData * );

    void itemChange(ItemChange change, const ItemChangeData & value);
    bool hasMaterial() { return m_material != NULL; }

private:
    void updateVideoStream();

signals:
    void frameDataChanged();
    void videoTypeChanged();
    void resolutionWidthChanged();
    void resolutionHeightChanged();
    void maskImageChanged();
    void maskBorderChanged();

private:
    int  m_windowID;
    int  m_resolutionWidth;
    int  m_resolutionHeight;

    float m_maskBorder;
    QRectF m_textureRect;
    QQuickItem* m_maskImage;
    QSGVideoMaterial *m_material;

    VideoStream* m_videoStream;

private Q_SLOTS:
    void videoImageUpdated();
};
QML_DECLARE_TYPE( VideoItem )

#endif // DEMO_VIDEOITEM_H
