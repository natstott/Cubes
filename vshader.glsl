#version 450 core


uniform mat4 project_matrix;
uniform mat4 translate_matrix;
uniform mat4 rotate_matrix;

uniform vec4 offsets[100];

in vec4 a_position;
in vec2 a_texcoord;

out vec2 v_texcoord;
out vec4 gl_Position;

//! [0]
void main()
{
    // Calculate vertex position in screen space
    vec4 offset = offsets[gl_InstanceID];

    // Vclip=Mproj.Mview.Mmodel.Vlocal

    gl_Position = project_matrix * translate_matrix * (rotate_matrix * a_position + offset);

    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces
    v_texcoord = a_texcoord;
}
//! [0]
