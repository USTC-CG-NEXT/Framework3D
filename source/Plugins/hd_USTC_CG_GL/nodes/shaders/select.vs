#version 430 core
#extension GL_EXT_gpu_shader4 : enable 


layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

//flat out uint vertexID;  // 传递顶点索引到片段着色器
//flat out uint primID;    // 传递物体ID到片段着色器

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
uniform int pID;

void main() {
    //vertexID = gl_VertexID;
    //primID = pID;
    gl_Position = projection * view * model * vec4(position, 1.0);
}