#pragma once

#define RGS(name)  extern "C" __global__ void __raygen__##name()
#define CHS(name)  extern "C" __global__ void __closesthit__##name()
#define MISS(name) extern "C" __global__ void __miss__##name()
#define IS(name)   extern "C" __global__ void __intersection__##name()
#define AHS(name)   extern "C" __global__ void __anyhit__##name()

#define STR(x)         #x
#define RGS_STR(name)    STR(__raygen__##name)
#define CHS_STR(name)    STR(__closesthit__##name)
#define MISS_STR(name)   STR(__miss__##name)
#define IS_STR(name)     STR(__intersection__##name)
#define AHS_STR(name)    STR(__anyhit__##name)

