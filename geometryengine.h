// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef GEOMETRYENGINE_H
#define GEOMETRYENGINE_H

#include <QOpenGLFunctions_4_5_Core> //max supported by 930MX
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>


class GeometryEngine : protected QOpenGLFunctions_4_5_Core
{
public:
    GeometryEngine();
    virtual ~GeometryEngine();

    void drawCubeGeometry(QOpenGLShaderProgram *program, int NumberOfDraws);
    void initCubeGeometry();
private:

    QOpenGLBuffer arrayBuf;
    QOpenGLBuffer indexBuf;
    //QOpenGLBuffer locationBuf; //positions of cubes
    //QOpenGLBuffer typeBuf; //types of cubes


};


#endif // GEOMETRYENGINE_H
