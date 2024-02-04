// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QApplication>
#include <QLabel>
#include <QSurfaceFormat>
#include <QOpenGLFunctions_4_5_Core> //max supported by 930MX

#ifndef QT_NO_OPENGL
#include "mainwidget.h"
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
        format.setVersion(4,5);
    QSurfaceFormat::setDefaultFormat(format);
    qDebug() <<format.majorVersion() <<"." <<format.minorVersion();
    app.setApplicationName("cube");
    app.setApplicationVersion("0.1");
#ifndef QT_NO_OPENGL


    MainWidget widget;
    widget.show();
#else
    QLabel note("OpenGL Support required");
    note.show();
#endif
    return app.exec();
}
