#version 430 core

uniform mat4 project_matrix;
uniform mat4 translate_matrix;
uniform mat4 rotate_matrix;

//uniform sampler2D texture1;
//uniform vec2 resolutionOfTexture;
//uniform int width;
//uniform int height;

//uniform vec4 offsets[100];
//struct ParticleData {
//    vec4 pos, dir, color;
//};

layout (std430, binding = 0) buffer Pos_Buf
{
    vec4 pos[];
};

layout (std430, binding = 4) buffer Draw_Buf
{
    uint draw_buf[];
};


in vec4 a_position;
in vec2 a_texcoord;

out vec2 v_texcoord;
out vec4 gl_Position;


void main()
{


// using buffer
    vec4 particle = pos[draw_buf[gl_InstanceID]]; // read particle number to draw from drawbuf and get its position
    vec4 offset=vec4(particle.xyz,0.0);
    int blocktype=int(particle.w)-1; //as image0 is first type, yet blocktype0=air?



    gl_Position = project_matrix * rotate_matrix *  translate_matrix * (a_position + offset);




    // Pass texture coordinate to fragment shader
    // Value will be automatically interpolated to fragments inside polygon faces. Blocktype selects which part to use.
    // should probably use 3Dtexture instead

    v_texcoord = vec2(a_texcoord.x/3+0.33333f*blocktype,a_texcoord.y);


}

