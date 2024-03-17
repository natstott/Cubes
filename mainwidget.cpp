// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
// texturepack minecraft blocks from https://github.com/Phyronnaz/VoxelAssets/tree/master/GoodVibes

/*
Good Vibes is a minecraft texture pack created from scratch by Acaitart.

It is licensed under the CC-BY license: you are free to use it in your own games, at the condition that you credit the original artist.

In other words, you need to add Acaitart to your game credits.

Also Cubed Textures under CC License
*/

#include "mainwidget.h"
#include <QMap>
#include <QMouseEvent>
#include <QPainter>
#include <cmath>
#include <QRgbaFloat32>
#include <QImageWriter>


MainWidget::~MainWidget()
{
    // Make sure the context is current when deleting the texture
    // and the buffers. AT THE END!
          qDebug() << "ending";
    setMouseTracking(true);
     static      QMap<int, bool> keys; //store if key is pressed
    keys[1]=false;
    makeCurrent();
    delete texture0;
    delete texture1;
    delete geometries;
    doneCurrent();
}



void MainWidget::initializeGL()
{
        qDebug() << "initialise";


    initializeOpenGLFunctions();
        qDebug() << "initialised";
    glClearColor(0.5, 0.5, 0.5, 1);
//compute shader set to 256,1,1
        mapwidth=512;
        mapheight=512;
        mapdepth=32;
        N=mapwidth*mapheight*mapdepth;

    geometries = new GeometryEngine;
    initShaders();
    initTextures();
    //debug max allowed buffer size 2GB
    GLint MaxBufSize;
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BLOCK_SIZE, &MaxBufSize);
    qDebug() << "Max buf: "<<MaxBufSize;
//qDebug() << "engine created";
    // Use QBasicTimer because its faster than QTimer
    timer.start(20, this);
}

//! [3]
void MainWidget::initShaders()
{
    // Compile vertex and fragment shader
    CubeGLprogram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vshader.glsl");
    CubeGLprogram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fshader.glsl");
    CubeGLprogram.link();
    CubeGLprogram.bind();

    geometries->initCubeGeometry();

    SkyBoxProgram.addShaderFromSourceFile(QOpenGLShader::Vertex,":/skybox.vert");
    SkyBoxProgram.addShaderFromSourceFile(QOpenGLShader::Fragment,":/skybox.frag");
    SkyBoxProgram.link();
    SkyBoxProgram.bind();

    // create draw cull compute shader
    DrawListProgram.addShaderFromSourceFile(QOpenGLShader::Compute, ":/makedrawlist.cs");
    DrawListProgram.link();
    DrawListProgram.bind();
    qDebug() << "Drawlist program" << DrawListProgram.isLinked();


//ComputeShader
    Computeprogram.addShaderFromSourceFile(QOpenGLShader::Compute, ":/compute.cs");
    Computeprogram.link();
    Computeprogram.bind();
    qDebug() << "compute program" << Computeprogram.isLinked();

    // make list of adjacent cubes for hidden cube culling and other comparison
    int adjacentcubes[26];
    int num=0;
    qDebug() << mapwidth <<mapheight << mapdepth;
    // Create map of adjacent cubes
    for(int z = -1; z < 2; z++)
    {
        for(int x = -1; x < 2; x++)
        {
            for(int y =-1; y<2;y++){
                if (!(y==0&&x==0&&z==0))
                {
                    adjacentcubes[num]=(z*mapwidth+x)*mapdepth+y;
                    //qDebug() << num << adjacentcubes[num] <<x << y << z;
                    num++;
                }

            }

        }
    }
    Computeprogram.setUniformValueArray("adjacentblocks", adjacentcubes,26);
    Computeprogram.setUniformValue("buf_size",N); //used to prevent read outside buffer.


    //generate arrays of blocks.

    QVector<QVector4D> pos, dir, col;//, color;
    QVector<uint> drawbuf;
// Create initial blocklayout and bind to buffers
    for(int z = 0; z < mapheight; z ++)
    {
        for(int x = 0; x < mapwidth; x ++)
        {
            //translation = QColor(x, 0, y, 0 ) ;
            float distance =sqrt(x*x+z*z);

            int height=5+2*cos(distance/7)+sin(x/11)-cos(z/13);

            //locationmap.setPixelColor(x, y, QColor(x,height,y,blocktype));
            for(int y =0; y<mapdepth;y++){
            int blocktype = (int(distance)/3+y)%3+1; //3 here is number of cubetypes - NEED TO DEFINE BETTER
                if (y>height)
                {
                    blocktype=0; // type 0 =air
                }
            pos.append(QVector4D(x,y,z,blocktype));
            dir.append(QVector4D(x,y,z,0));
            col.append(QVector4D(1,1,1, (blocktype==0) ? 0.01:1.0));
            drawbuf.append(0);
            }

        }
    }
    // counter buffer for atomic counter for culling in draw shader
    QVector<int> buffer_Counter;
    for (int i=0; i<3; i++) {
        buffer_Counter.append(0);
    }



    // create and bind buffers to shaders - it appears the binding number is consisten across all shaders.
    CubeGLprogram.bind();

    Pos_Buf.create();
    Pos_Buf.bind();
    Pos_Buf.allocate(pos.constData(), pos.size()*sizeof(QVector4D));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, Pos_Buf.bufferId());

    Dir_Buf.create();
    Dir_Buf.bind();
    Dir_Buf.allocate(dir.constData(), dir.size()*sizeof(QVector4D));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, Dir_Buf.bufferId());

    Col_Buf.create();
    Col_Buf.bind();
    Col_Buf.allocate(col.constData(), col.size()*sizeof(QVector4D));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, Col_Buf.bufferId());

    // bind color buffer used for selecting visible, counter buffer and draw buffer to shader
    Counter_Buf.create();
    Counter_Buf.bind();
    Counter_Buf.allocate(buffer_Counter.constData(),sizeof(buffer_Counter));
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 3, Counter_Buf.bufferId());

    Draw_Buf.create();
    Draw_Buf.bind();
    Draw_Buf.allocate(drawbuf.constData(),drawbuf.size()*sizeof(uint));
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER,4,Draw_Buf.bufferId());

}


void MainWidget::initTextures()
{
    // Load cube.png image

    int imagew = QImage(":/textures/blocks/grass_top.png").width();
    int imageh = QImage(":/textures/blocks/grass_top.png").height();

    QList<QStringList> images;
    images.append({":/textures/blocks/grass_path_side.png",":/textures/blocks/grass_path_top.png", ":/textures/blocks/grass_path_side.png",
                       ":/textures/blocks/grass_path_side.png",":/textures/blocks/dirt.png",":/textures/blocks/grass_path_side.png"});

    images.append({":/textures/blocks/quartz_block_side.png",":/textures/blocks/quartz_block_top.png",":/textures/blocks/quartz_block_side.png",
                   ":/textures/blocks/quartz_block_side.png",":/textures/blocks/quartz_block_bottom.png",":/textures/blocks/quartz_block_side.png"});

    images.append({":/textures/blocks/red_sand.png",":/textures/blocks/red_sandstone_top.png",":/textures/blocks/red_sand.png",
                   ":/textures/blocks/red_sand.png",":/textures/blocks/red_sandstone_bottom.png",":/textures/blocks/red_sand.png"});

    //images.append({":/textures/blocks/stone_granite.png",":/textures/blocks/stone_granite.png",":/textures/blocks/stone_granite.png",
      //             ":/textures/blocks/stone_granite.png",":/textures/blocks/stone_granite.png",":/textures/blocks/stone_granite.png"});
    int numImages=images.size();
     QImage minecraftmap(3*imagew*numImages,2*imageh,QImage::Format_ARGB32);

    // texture order back, bottom, left, right, top, front
     //create a texture maps of 3W*2H cube textures. map is as wide as number of texture cubes
    QPainter painter(&minecraftmap);
     for(int i=0;i<numImages;i++){
    painter.drawImage(0+3*imagew*i,0,QImage(images[i][0]));
         painter.drawImage(imagew*(3*i+1),0,QImage(images[i][1]));
    painter.drawImage(imagew*(3*i+2),0,QImage(images[i][2]));
    painter.drawImage(0+3*imagew*i,imageh,QImage(images[i][3]));
    painter.drawImage(imagew*(3*i+1),imageh,QImage(images[i][4]));
    painter.drawImage(imagew*(3*i+2),imageh,QImage(images[i][5]));
    //qDebug() << "image" << i <<QImage(images[i][0]).width();
     }
    texture0 = new QOpenGLTexture(QImage(minecraftmap).mirrored());

     QImageWriter writer("/home/nat/Documents/build/testimage/textures.jpg", "JPG"); //for debugging image
   writer.write(minecraftmap);
    // Set nearest filtering mode for texture minification
    texture0->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification

    texture0->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    texture0->setWrapMode(QOpenGLTexture::Repeat);

/*
    //Format_RGBX32FPx4 Using texture to store data
    QImage locationmap(mapwidth,mapheight,QImage::Format_ARGB32);

    //QColor translation ;
    for(int y = 0; y < mapheight; y ++)
    {
        for(int x = 0; x < mapwidth; x ++)
        {
            //translation = QColor(x, 0, y, 0 ) ;
            float distance =sqrt(x*x+y*y);

            int height=3+cos(distance/7)+sin(x/11)-cos(y/13);
            int blocktype = (int(distance)/3+height)%numImages;
            locationmap.setPixelColor(x, y, QColor(x,height,y,blocktype));


        }
    }
    //texture1->setFormat(QOpenGLTexture::RGBA32F);

    texture1 = new QOpenGLTexture(locationmap.mirrored());
    qDebug() <<texture1->format();
    // Set nearest filtering mode for texture minification
    texture1->setMinificationFilter(QOpenGLTexture::Nearest);

    // Set bilinear filtering mode for texture magnification
    texture1->setMagnificationFilter(QOpenGLTexture::Linear);

    // Wrap texture coordinates by repeating
    // f.ex. texture coordinate (1.1, 1.2) is same as (0.1, 0.2)
    texture1->setWrapMode(QOpenGLTexture::Repeat);
*/

    MakeSkyBox(); //creates skybox texture "skybox"

}



void MainWidget::resizeGL(int w, int h)
{
    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 1.01, zFar = 250.0, fov = 45.0;

    // Reset projection
    cameraPerspective.setToIdentity();

    // Set perspective
    cameraPerspective.perspective(fov, aspect, zNear, zFar);
    cameraLocation+=QVector3D(-25,-5,-25);

    skyboxPerspective.setToIdentity();
    skyboxPerspective.perspective(fov,aspect,0.1,2.0);

}
//! [5]

void MainWidget::paintGL()
{

    // Clear color and depth buffer start of draw routine
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);

    // Enable back face culling
    glEnable(GL_CULL_FACE);

    // Calculate model view transformation

    QMatrix4x4 translatematrix=cameraAngle;
//    QMatrix4x4 projection;

    translatematrix.translate(cameraLocation);
    //translatematrix*=cameraAngle;

    //try looking using mouseposition
    QPoint  winXY = QCursor::pos();
    float h = 1.0f*winXY.y()/  this->height()-0.5;
    float w = 1.0f*winXY.x()/ this->width()-0.5;
    float lookAngle=120*sqrt(h*h+w*w);

    rotatematrix.setToIdentity();
    rotatematrix.rotate(lookAngle,h/lookAngle, w/lookAngle, 0.0f);

    glActiveTexture(GL_TEXTURE0);
    texture0->bind();

    glActiveTexture(GL_TEXTURE2);
    skybox->bind();
    DrawSkyBox(skyboxPerspective*rotatematrix*cameraAngle);

    runcomputeshaders();
    CubeGLprogram.bind(); // vertex and frag shader can use texture for storage instead of buffer
    CubeGLprogram.setUniformValue("rotate_matrix", rotatematrix);
    CubeGLprogram.setUniformValue("translate_matrix", translatematrix);
    CubeGLprogram.setUniformValue("project_matrix", cameraPerspective);

    Counter_Buf.bind();
    int readbackdata[3];
    Counter_Buf.read(0,readbackdata, sizeof(readbackdata));
    // Draw cube geometry
    geometries->drawCubeGeometry(&CubeGLprogram,readbackdata[0]);

}

void MainWidget::runcomputeshaders()
{
    // main compute shader to process data
    float coeff=1;//cos(ticks/60); //was raising and lowering as test
    Computeprogram.bind();
    Computeprogram.setUniformValue("coeff", (coeff)); //set uniform
    //qDebug() << ticks << coeff;
    glDispatchCompute(N/256,1,1);
    Computeprogram.release();


    // compute shader to purge hidden blocks
    DrawListProgram.bind();
    //DrawListProgram.setUniformValue("Count",N); //loop wont work.
    Counter_Buf.bind();
    //set to zero
    QVector<int> buffer_Counter;
    for (int i=0; i<3; i++) {
        buffer_Counter.append(0);
        //qDebug() << "  " << buffer_Counter[i];
    }
    Counter_Buf.allocate(buffer_Counter.constData(),sizeof(buffer_Counter));
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 3, Counter_Buf.bufferId());

    //glDispatchCompute(1,1,1);
    glDispatchCompute(N/256,1,1);
    DrawListProgram.release();

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
    // Mouse release position - mouse press position - not used
    /*
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
*/
}

// store currently pressed keys in Qmap to use for moevement.
void MainWidget::keyPressEvent(QKeyEvent *e)
{ keys[e->key()] = true;
    QWidget::keyPressEvent(e);
    //qDebug() << "key" << e->key();
}

void MainWidget::keyReleaseEvent(QKeyEvent *e)
{ keys[e->key()] = false;
    QWidget::keyReleaseEvent(e); }


//! timerEvent here is effectively used as game tick
//!
void MainWidget::timerEvent(QTimerEvent *)
{

    DoCameraMovement();

    //Run simulations ....
    ticks++;

    // Request an update
    update();
}
//! [1]

void MainWidget::DoCameraMovement(){

float movespeed=0.5;
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

}
void MainWidget::MakeSkyBox()
{
QStringList    faces
    {
        ":/textures/skybox/right.jpg",
        ":/textures/skybox/top.jpg",
        ":/textures/skybox/front.jpg",
            ":/textures/skybox/left.jpg",
         ":/textures/skybox/bottom.jpg",
           ":/textures/skybox/back.jpg"
    };

    QImage skyboxes[6];

    for (unsigned int i = 0; i < faces.size(); i++)
    {
        skyboxes[i] = QImage((faces[i])).convertToFormat(QImage::Format_RGBA8888);
        //qDebug() << skyboxes[i].width();
        //QImageWriter writer("/home/nat/Documents/build/testimage/image.jpg", "JPG");
        //writer.write(skyboxes[i]);

    }


    skybox = new QOpenGLTexture(QOpenGLTexture::TargetCubeMap);
    skybox->create();
    skybox->setSize(skyboxes[0].width(), skyboxes[0].height(), skyboxes[0].depth());
    skybox->setFormat(QOpenGLTexture::RGBA8_UNorm);
    skybox->allocateStorage();

    skybox->setData(0, 0, QOpenGLTexture::CubeMapPositiveX,
                     QOpenGLTexture::RGBA, QOpenGLTexture::UInt8,
                    skyboxes[0].constBits(), Q_NULLPTR);

    skybox->setData(0, 0, QOpenGLTexture::CubeMapPositiveY,
                     QOpenGLTexture::RGBA, QOpenGLTexture::UInt8,
                     skyboxes[1].constBits(), Q_NULLPTR);

    skybox->setData(0, 0, QOpenGLTexture::CubeMapPositiveZ,
                     QOpenGLTexture::RGBA, QOpenGLTexture::UInt8,
                     skyboxes[2].constBits(), Q_NULLPTR);

    skybox->setData(0, 0, QOpenGLTexture::CubeMapNegativeX,
                     QOpenGLTexture::RGBA, QOpenGLTexture::UInt8,
                     skyboxes[3].constBits(), Q_NULLPTR);

    skybox->setData(0, 0, QOpenGLTexture::CubeMapNegativeY,
                     QOpenGLTexture::RGBA, QOpenGLTexture::UInt8,
                     skyboxes[4].constBits(), Q_NULLPTR);

    skybox->setData(0, 0, QOpenGLTexture::CubeMapNegativeZ,
                     QOpenGLTexture::RGBA, QOpenGLTexture::UInt8,
                     skyboxes[5].constBits(), Q_NULLPTR);
   skybox->setWrapMode(QOpenGLTexture::ClampToEdge);
   skybox->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
   skybox->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
   //qDebug() << "target" << skybox->width();
   //qDebug() << "format" << skybox->format();
   skybox->bind();

}

void MainWidget::DrawSkyBox(QMatrix4x4 mProjectionMat)
{

    SkyBoxProgram.bind();
    glActiveTexture(GL_TEXTURE2);
    skybox->bind();


    SkyBoxProgram.setUniformValue("mvpMatrix", mProjectionMat);

    QVector3D vertices[] =
        {
            {-1.0f,  1.0f, -1.0f},
            {-1.0f, -1.0f, -1.0f},
            {+1.0f, -1.0f, -1.0f},
            {+1.0f, -1.0f, -1.0f},
            {+1.0f, +1.0f, -1.0f},
            {-1.0f, +1.0f, -1.0f},

            {-1.0f, -1.0f, +1.0f},
            {-1.0f, -1.0f, -1.0f},
            {-1.0f, +1.0f, -1.0f},
            {-1.0f, +1.0f, -1.0f},
            {-1.0f, +1.0f, +1.0f},
            {-1.0f, -1.0f, +1.0f},

            {+1.0f, -1.0f, -1.0f},
            {+1.0f, -1.0f, +1.0f},
            {+1.0f, +1.0f, +1.0f},
            {+1.0f, +1.0f, +1.0f},
            {+1.0f, +1.0f, -1.0f},
            {+1.0f, -1.0f, -1.0f},

            {-1.0f, -1.0f, +1.0f},
            {-1.0f, +1.0f, +1.0f},
            {+1.0f, +1.0f, +1.0f},
            {+1.0f, +1.0f, +1.0f},
            {+1.0f, -1.0f, +1.0f},
            {-1.0f, -1.0f, +1.0f},

            {-1.0f, +1.0f, -1.0f},
            {+1.0f, +1.0f, -1.0f},
            {+1.0f, +1.0f, +1.0f},
            {+1.0f, +1.0f, +1.0f},
            {-1.0f, +1.0f, +1.0f},
            {-1.0f, +1.0f, -1.0f},

            {-1.0f, -1.0f, -1.0f},
            {-1.0f, -1.0f, +1.0f},
            {+1.0f, -1.0f, -1.0f},
            {+1.0f, -1.0f, -1.0f},
            {-1.0f, -1.0f, +1.0f},
            {+1.0f, -1.0f, +1.0f}
        };

    QOpenGLBuffer mVertexBuf;
    mVertexBuf.create();
    mVertexBuf.bind();
    mVertexBuf.allocate(vertices, 36 * sizeof(QVector3D));

    glDepthMask(GL_FALSE);

    SkyBoxProgram.enableAttributeArray("aPosition");
    SkyBoxProgram.setAttributeBuffer("aPosition",  GL_FLOAT,  0, 3, sizeof(QVector3D));

    glDrawArrays(GL_TRIANGLES,
                 0,
                 36);
    glDepthMask(GL_TRUE);

}
