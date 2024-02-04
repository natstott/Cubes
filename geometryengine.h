// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef GEOMETRYENGINE_H
#define GEOMETRYENGINE_H

#include <QOpenGLFunctions_4_5_Compatibility> //max supported by 930MX
//#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

#include <QOpenGLExtraFunctions>

class GeometryEngine : protected QOpenGLFunctions_4_5_Compatibility
{
public:
    GeometryEngine();
    virtual ~GeometryEngine();

    void drawCubeGeometry(QOpenGLShaderProgram *program);

private:
    void initCubeGeometry();
    void initCubeLocations();
    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
    QOpenGLBuffer locationBuf;


};


#endif // GEOMETRYENGINE_H
