#version 430

layout (std430, binding = 2) buffer Col_Buf
{
    vec4 col[];
};


layout(binding = 3, offset = 0) uniform atomic_uint index;
//Buffers for processing - Col_Buf to identify which cubes are not drawn? E.g transparency <0.1
layout (std430, binding = 4) buffer Draw_Buf
{
    uint draw_buf[];
};

//uniform int count;



layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

void main(void)
{
    uint id = gl_GlobalInvocationID.x;

//for (int i=0; i<count;i++){  //loop wont work, have to use warps
//uint id=uint(i);
if(col[id].a > 0.1) // using col.a=0 to represent invisible cubes.
    {
    uint oldIndex = atomicCounterIncrement(index);
    draw_buf[oldIndex] = id;

    }
//}
}
