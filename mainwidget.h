// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "geometryengine.h"
#include "qopenglfunctions_4_3_core.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_3_Core> //max supported by 930MX
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QBasicTimer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

class GeometryEngine;

class MainWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core
{
    Q_OBJECT

public:
    using QOpenGLWidget::QOpenGLWidget;
    ~MainWidget();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;

    void timerEvent(QTimerEvent *e) override;

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void runcomputeshaders();
    void initShaders();
    void initTextures();
    void DoCameraMovement();
    void MakeSkyBox();
    void DrawSkyBox(QMatrix4x4 mProjectionMat);




private:
    int mapwidth,mapheight,mapdepth;
    QBasicTimer timer;
    QOpenGLShaderProgram CubeGLprogram , SkyBoxProgram, Computeprogram, DrawListProgram;
    QOpenGLBuffer Pos_Buf, Dir_Buf, Col_Buf, Draw_Buf, Counter_Buf;
    GeometryEngine *geometries = nullptr;
    QMap<int, bool> keys; //store if key is pressed
    QOpenGLTexture *texture0 = nullptr;
    QOpenGLTexture *texture1 = nullptr;
    QOpenGLTexture *skybox =nullptr;

    QVector3D cameraLocation;
    QMatrix4x4 cameraAngle;
    QMatrix4x4 cameraPerspective;
    QMatrix4x4 skyboxPerspective;
    QMatrix4x4 rotatematrix; //used for mouselook

    QVector2D mousePressPosition;
    QVector3D rotationAxis;
    qreal angularSpeed = 0;
    QQuaternion rotation;
    float ticks;
    int N; //number of blocks
};

#endif // MAINWIDGET_H
