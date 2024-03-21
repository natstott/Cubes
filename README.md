# Cubes
QT OpenGl Instanced cubes

This started with the QT opengl example cube, upgraded to opengl4.3, to allow instancing of cubes.

Cubes are drawn using instancing from SSBO buffer.

Compute shader can manipulate cubes

Culling implemented by identifying hidden cubes

All done as a painful experiment in using opengl directly instead of Unity!

Cannot get compute shaders to run properly outside GLpaint thread at the moment. As one of them is a draw culling shader thats fine, but the other one will be a simulation/physics process.



