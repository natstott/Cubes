#version 450 core
in vec3 aPosition;


out vec3 vTexCoord;
uniform mat4 mvpMatrix;

void main()
{
    gl_Position = mvpMatrix * vec4(aPosition, 1.0);
    //gl_Position = Position.xyww;
    vTexCoord = aPosition;
}
