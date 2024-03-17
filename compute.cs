#version 430

//Example of uniform parameter
uniform float coeff=1;

//Buffers for processing
layout (std430, binding = 0) buffer Pos_Buf
{
    vec4 pos_buf[];
};

layout (std430, binding = 1) buffer Dir_Buf
{
    vec4 dir_buf[];
};

//Example of using structure:
//struct ParticleData {
//    vec4 pos, dir, color;
//};
layout (std430, binding = 2) buffer Col_Buf
{
    vec4 col[];
};

uniform int[26] adjacentblocks;
uniform uint buf_size;

layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

void main(void)
{
    uint id = gl_GlobalInvocationID.x;



//update positions

//Check neighbors for gaps visible pos.w=blocktype, 0=air
bool hidden=true;
for(int i =0;i<26;i++)
{
int testpos= int(id)+adjacentblocks[i];
if (testpos>-1 && testpos<buf_size-1)
{
 if (pos_buf[testpos].w==0) hidden=false;
}
else hidden=false;
}

if(hidden) col[id].a=0.0;
vec4 postemp=pos_buf[id];//vec4(pos_buf[id].x,dir_buf[id].y*coeff,pos_buf[id].z,pos_buf[id].w);
pos_buf[id] = postemp;
}


