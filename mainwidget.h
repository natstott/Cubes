// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "geometryengine.h"

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Compatibility> //max supported by 930MX
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QBasicTimer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

class GeometryEngine;

class MainWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Compatibility
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

    void initShaders();
    void initTextures();




private:
    QBasicTimer timer;
    QOpenGLShaderProgram program;
    GeometryEngine *geometries = nullptr;
    QMap<int, bool> keys; //store if key is pressed
    QOpenGLTexture *texture = nullptr;

    QVector3D cameraLocation;
    QMatrix4x4 cameraAngle;
    QMatrix4x4 cameraPerspective;

    QVector2D mousePressPosition;
    QVector3D rotationAxis;
    qreal angularSpeed = 0;
    QQuaternion rotation;
};

#endif // MAINWIDGET_H
