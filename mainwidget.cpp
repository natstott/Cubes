// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// texturepack minecraft blocks from https://github.com/Phyronnaz/VoxelAssets/tree/master/GoodVibes
/*
Good Vibes is a minecraft texture pack created from scratch by Acaitart.

It is licensed under the CC-BY license: you are free to use it in your own games, at the condition that you credit the original artist.

In other words, you need to add Acaitart to your game credits.
*/

#include "mainwidget.h"
#include <QMap>
#include <QMouseEvent>
#include <QPainter>
#include <cmath>

MainWidget::~MainWidget()
{
    // Make sure the context is current when deleting the texture
    // and the buffers.
          qDebug() << "makecurrent";
    setMouseTracking(true);
     static      QMap<int, bool> keys; //store if key is pressed
    keys[1]=false;
    makeCurrent();
    delete texture;
    delete geometries;
    doneCurrent();
}

//Input handling

//! [0]
void MainWidget::mousePressEvent(QMouseEvent *e)
{
    // Save mouse press position
    mousePressPosition = QVector2D(e->position());
}

void MainWidget::mouseReleaseEvent(QMouseEvent *e)
{
    // Mouse release position - mouse press position
    QVector2D diff = QVector2D(e->position()) - mousePressPosition;

    // Rotation axis is perpendicular to the mouse position difference
    // vector
    QVector3D n = QVector3D(diff.y(), diff.x(), 0.0).normalized();

    // Accelerate angular speed relative to the length of the mouse sweep
    qreal acc = diff.length() / 100.0;

    // Calculate new rotation axis as weighted sum
    rotationAxis = (rotationAxis * angularSpeed + n * acc).normalized();

    // Increase angular speed
    angularSpeed += acc;
}

void MainWidget::keyPressEvent(QKeyEvent *e)
{ keys[e->key()] = true;
    QWidget::keyPressEvent(e);
    //qDebug() << "key" << e->key();
}

void MainWidget::keyReleaseEvent(QKeyEvent *e)
{ keys[e->key()] = false;
    QWidget::keyReleaseEvent(e); }



//! [0]

//! [1]
//! timerEvent here is effectively used as game tick
//!
void MainWidget::timerEvent(QTimerEvent *)
{
    // Decrease angular speed (friction)
    angularSpeed *= 0.99;

    // Stop rotation when speed goes below threshold
    if (angularSpeed < 0.01) {
        angularSpeed = 0.0;
    } else {
        // Update rotation
        rotation = QQuaternion::fromAxisAndAngle(rotationAxis, angularSpeed) * rotation;

    }

    //keymovements
    /*
Will need to abstract this to different controllers

*/
    float movespeed=0.1;
    float rotatespeed = 1;
        QVector3D movecameramatrix;

    if(keys[Qt::Key_Up]){
        movecameramatrix += QVector3D(0.0,0.0,1.0);
    }
    if(keys[Qt::Key_Down]){
        movecameramatrix += QVector3D(0.0,0.0,-1.0);
    }
    if(keys[Qt::Key_PageDown]){
        movecameramatrix += QVector3D(0.0,-1.0,0.0);
    }
    if(keys[Qt::Key_PageUp]){
        movecameramatrix += QVector3D(0.0,1.0,0.0);
    }
    if(keys[Qt::Key_Comma]){
        movecameramatrix += QVector3D(1.0,0.0,0.0);
    }    if(keys[Qt::Key_Period]){
        movecameramatrix += QVector3D(-1.0,0.0,0.0);
    }

    if(keys[Qt::Key_Left]){
        cameraAngle.rotate(-1.0*rotatespeed,0.0,1.0,0.0);
    }
    if(keys[Qt::Key_Right]){
        cameraAngle.rotate(1.0*rotatespeed,0.0,1.0,0.0);
    }


    cameraLocation+= (cameraAngle.inverted().map(movecameramatrix*movespeed));

    // Request an update
    update();
}
//! [1]

void MainWidget::initializeGL()
{
        qDebug() << "initialise";


    initializeOpenGLFunctions();
        qDebug() << "initialised";
    glClearColor(0.5, 0.5, 0.5, 1);

    initShaders();
    initTextures();

    geometries = new GeometryEngine;
qDebug() << "engine created";
    // Use QBasicTimer because its faster than QTimer
    timer.start(12, this);
}

//! [3]
void MainWidget::initShaders()
{
    // Compile vertex shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl"))
        close();

    // Compile fragment shader
    if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl"))
        close();

    // Link shader pipeline
    if (!program.link())
        close();

    // Bind shader pipeline for use
    if (!program.bind())
        close();





}
//! [3]

//! [4]
void MainWidget::initTextures()
{
    // Load cube.png image

    qDebug() <<"image" << QImage(":/textures/block/grass_block_top.png").width()<<","<<QImage(":/textures/block/grass_block_top.png").height();
    QImage minecraftmap(48,32,QImage::Format_ARGB32);
    QStringList images({":/textures/block/grass_path_side.png",":/textures/block/grass_block_top.png",":/textures/block/grass_path_side.png",
                        ":/textures/block/grass_path_side.png",":/textures/block/dirt.png",":/textures/block/grass_path_side.png"});

    QPainter painter(&minecraftmap);
    painter.drawImage(0,0,QImage(images[0]));
    painter.drawImage(16,0,QImage(images[1]));
    painter.drawImage(32,0,QImage(images[2]));
    painter.drawImage(0,16,QImage(images[3]));
    painter.drawImage(16,16,QImage(images[4]));
    painter.drawImage(32,16,QImage(images[5]));

    texture = new QOpenGLTexture(QImage(minecraftmap).mirrored());
    //    texture = new QOpenGLTexture(QImage(":/minecraft.png").mirrored());

    // Set nearest filtering mode for texture minification
    texture->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    texture->setWrapMode(QOpenGLTexture::Repeat);
}
//! [4]

//! [5]
void MainWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 0.3, zFar = 20.0, fov = 45.0;

    // Reset projection
    cameraPerspective.setToIdentity();

    // Set perspective projection
    cameraPerspective.perspective(fov, aspect, zNear, zFar);
    cameraLocation+=QVector3D(0,-1,0);
}
//! [5]

void MainWidget::paintGL()
{

    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//! [2]
    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glEnable(GL_CULL_FACE);
//! [2]

    texture->bind();
    program.bind();

//! [6]
    // Calculate model view transformation
    QMatrix4x4 rotatematrix; //just rotates cubes in space
    QMatrix4x4 translatematrix=cameraAngle;


    /*//try moving using mouseposition
    QPoint  winXY = QCursor::pos();
    float h =1.0f*winXY.y()/  this->height()-0.5;
    float w = 1.0f*winXY.x()/ this->width()-0.5;*/
    QMatrix4x4 projection;
    translatematrix.translate(cameraLocation);
    //translatematrix*=cameraAngle;


    rotatematrix.rotate(rotation); //spinningcubes

    // Set modelview-projection matrix
    program.setUniformValue("rotate_matrix", rotatematrix);
    program.setUniformValue("translate_matrix", translatematrix);


    program.setUniformValue("project_matrix", cameraPerspective);

//! [6]
/* Used to test instancing
   QMatrix4x4 offsets = QMatrix4x4{
        0.3f,-0.3f,-0.3f,0.3f,
        -0.3f,0.3f,0.3f,-0.3f,
        0.3f,-0.3f,0.3f,-0.3f,
        0.0f,0.0f,0.0f,0.0f
    };

    program.setUniformValue("offsets2", offsets);
*/

    QVector4D translations[100];
    int index = 0;

    for(int y = -5; y < 5; y ++)
    {
        for(int x = -5; x < 5; x ++)
        {
            QVector4D translation = QVector4D(
                (float)x  + 0.5f ,
                0.0,
                (float)y + 0.5f,
                 0.0f) ;
            translations[index++] = translation;
        }
    }
    //write values to shader
    program.setUniformValueArray(("offsets[]"), translations,100);



 /*   //try scaling using mouseposition
    QPoint  winXY = QCursor::pos();
    float h =1.0f*winXY.y()/  this->height();
    float w = 1.0f*winXY.x()/ this->width();

    QVector2D mousescale = QVector2D(w,h);
    program.setUniformValue("scales", mousescale);

    // Use texture unit 0 which contains cube.png
    program.setUniformValue("texture", 0);
*/
    // Draw cube geometry
    geometries->drawCubeGeometry(&program);

}

