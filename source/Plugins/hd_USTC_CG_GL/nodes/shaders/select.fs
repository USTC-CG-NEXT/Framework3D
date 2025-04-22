#version 430
#extension GL_EXT_GPU_SHADER4 : enable

layout(location = 0) out int rst;

//flat in uint vertexID; 
//flat in uint primID;   

void main() {
    //uint safePrimID = primID + 1;
    //uint safeVertexID = vertexID;

    rst = 1000;
}