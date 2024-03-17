#version 430 core


uniform sampler2D texture0;


in vec2 v_texcoord;
out vec4 Color;

//! [0]
void main()
{
    // Set fragment color from texture
    Color = texture2D(texture0, v_texcoord);
}
//! [0]

