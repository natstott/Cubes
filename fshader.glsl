#version 450 core
#ifdef GL_ES
// Set default precision to medium
precision mediump in;
precision mediump float;
#endif

uniform sampler2D texture;

in vec2 v_texcoord;
out vec4 Color;

//! [0]
void main()
{
    // Set fragment color from texture
    Color = texture2D(texture, v_texcoord);
}
//! [0]

