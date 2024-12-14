#include "basic_node_base.h"

NODE_DEF_OPEN_SCOPE
static constexpr std::string socket_name(int i)
{
    switch (i) {
        case 0: return "X";
        case 1: return "Y";
        case 2: return "Z";
        case 3: return "W";
        default: return "FF";
    }
}

NODE_DECLARATION_FUNCTION(create_buffer1f)
{
    for (int i = 0; i < 1; ++i) {
        b.add_input<float>(socket_name(i).c_str())
            .min(-10)
            .max(10)
            .default_val(0);
    }
    b.add_input<int>("Size").min(1).max(200).default_val(1);
    b.add_output<float1Buffer>("Buffer");
};
NODE_DECLARATION_FUNCTION(create_buffer2f)
{
    for (int i = 0; i < 2; ++i) {
        b.add_input<float>(socket_name(i).c_str())
            .min(-10)
            .max(10)
            .default_val(0);
    }
    b.add_input<int>("Size").min(1).max(200).default_val(1);
    b.add_output<float2Buffer>("Buffer");
};
NODE_DECLARATION_FUNCTION(create_buffer3f)
{
    for (int i = 0; i < 3; ++i) {
        b.add_input<float>(socket_name(i).c_str())
            .min(-10)
            .max(10)
            .default_val(0);
    }
    b.add_input<int>("Size").min(1).max(200).default_val(1);
    b.add_output<float3Buffer>("Buffer");
};
NODE_DECLARATION_FUNCTION(create_buffer4f)
{
    for (int i = 0; i < 4; ++i) {
        b.add_input<float>(socket_name(i).c_str())
            .min(-10)
            .max(10)
            .default_val(0);
    }
    b.add_input<int>("Size").min(1).max(200).default_val(1);
    b.add_output<float4Buffer>("Buffer");
};

NODE_DECLARATION_FUNCTION(create_float3f)
{
    for (int i = 0; i < 3; ++i) {
        b.add_input<float>(socket_name(i).c_str())
            .min(-10)
            .max(10)
            .default_val(0);
    }
    b.add_output<pxr::GfVec3f>("Buffer");
};
NODE_DECLARATION_FUNCTION(create_int3)
{
    for (int i = 0; i < 3; ++i) {
        b.add_input<int>(socket_name(i).c_str())
            .min(-10)
            .max(10)
            .default_val(0);
    }
    b.add_output<pxr::GfVec3i>("Buffer");
};

NODE_DECLARATION_FUNCTION(create_buffer1i)
{
    for (int i = 0; i < 1; ++i) {
        b.add_input<int>(socket_name(i).c_str())
            .min(-10)
            .max(10)
            .default_val(0);
    }
    b.add_input<int>("Size").min(1).max(200).default_val(1);
    b.add_output<int1Buffer>("Buffer");
};
NODE_DECLARATION_FUNCTION(create_buffer2i)
{
    for (int i = 0; i < 2; ++i) {
        b.add_input<int>(socket_name(i).c_str())
            .min(-10)
            .max(10)
            .default_val(0);
    }
    b.add_input<int>("Size").min(1).max(200).default_val(1);
    b.add_output<int2Buffer>("Buffer");
};
NODE_DECLARATION_FUNCTION(create_buffer3i)
{
    for (int i = 0; i < 3; ++i) {
        b.add_input<int>(socket_name(i).c_str())
            .min(-10)
            .max(10)
            .default_val(0);
    }
    b.add_input<int>("Size").min(1).max(200).default_val(1);
    b.add_output<int3Buffer>("Buffer");
};
NODE_DECLARATION_FUNCTION(create_buffer4i)
{
    for (int i = 0; i < 4; ++i) {
        b.add_input<int>(socket_name(i).c_str())
            .min(-10)
            .max(10)
            .default_val(0);
    }
    b.add_input<int>("Size").min(1).max(200).default_val(1);
    b.add_output<int4Buffer>("Buffer");
};

#define NodeExec(type, base_type, suffix, size)                       \
    static void node_create_buffer##size##suffix(ExeParams params)    \
    {                                                                 \
        base_type val[size];                                          \
        for (int i = 0; i < size; ++i) {                              \
            val[i] = params.get_input<float>(socket_name(i).c_str()); \
        }                                                             \
        auto s = params.get_input<int>("Size");                       \
                                                                      \
        type data;                                                    \
        memcpy(&data, val, sizeof(type));                             \
                                                                      \
        pxr::VtArray<type> arr;                                       \
        arr.resize(s, data);                                          \
        params.set_output("Buffer", arr);                             \
    }
NODE_EXECUTION_FUNCTION(create_buffer1f)
{
    float val[1];
    for (int i = 0; i < 1; ++i) {
        val[i] = params.get_input<float>(socket_name(i).c_str());
    }
    auto s = params.get_input<int>("Size");
    float data;
    memcpy(&data, val, sizeof(float));
    pxr::VtArray<float> arr;
    arr.resize(s, data);
    params.set_output("Buffer", arr);
    return true;
};
NODE_EXECUTION_FUNCTION(create_buffer2f)
{
    float val[2];
    for (int i = 0; i < 2; ++i) {
        val[i] = params.get_input<float>(socket_name(i).c_str());
    }
    auto s = params.get_input<int>("Size");
    pxr::GfVec2f data;
    memcpy(&data, val, sizeof(pxr::GfVec2f));
    pxr::VtArray<pxr::GfVec2f> arr;
    arr.resize(s, data);
    params.set_output("Buffer", arr);
    return true;
};
NODE_EXECUTION_FUNCTION(create_buffer3f)
{
    float val[3];
    for (int i = 0; i < 3; ++i) {
        val[i] = params.get_input<float>(socket_name(i).c_str());
    }
    auto s = params.get_input<int>("Size");
    pxr::GfVec3f data;
    memcpy(&data, val, sizeof(pxr::GfVec3f));
    pxr::VtArray<pxr::GfVec3f> arr;
    arr.resize(s, data);
    params.set_output("Buffer", arr);
    return true;
};
NODE_EXECUTION_FUNCTION(create_buffer4f)
{
    float val[4];
    for (int i = 0; i < 4; ++i) {
        val[i] = params.get_input<float>(socket_name(i).c_str());
    }
    auto s = params.get_input<int>("Size");
    pxr::GfVec4f data;
    memcpy(&data, val, sizeof(pxr::GfVec4f));
    pxr::VtArray<pxr::GfVec4f> arr;
    arr.resize(s, data);
    params.set_output("Buffer", arr);
    return true;
};

NODE_EXECUTION_FUNCTION(create_float3f)
{
    float val[3];
    for (int i = 0; i < 3; ++i) {
        val[i] = params.get_input<float>(socket_name(i).c_str());
    }
    pxr::GfVec3f data;
    memcpy(&data, val, sizeof(pxr::GfVec3f));
    params.set_output("Buffer", data);
    return true;
};
NODE_EXECUTION_FUNCTION(create_int3)
{
    float val[3];
    for (int i = 0; i < 3; ++i) {
        val[i] = params.get_input<int>(socket_name(i).c_str());
    }
    pxr::GfVec3i data;
    memcpy(&data, val, sizeof(pxr::GfVec3i));
    params.set_output("Buffer", data);
    return true;
};

NODE_EXECUTION_FUNCTION(create_buffer1i)
{
    int val[1];
    for (int i = 0; i < 1; ++i) {
        val[i] = params.get_input<float>(socket_name(i).c_str());
    }
    auto s = params.get_input<int>("Size");
    int data;
    memcpy(&data, val, sizeof(int));
    pxr::VtArray<int> arr;
    arr.resize(s, data);
    params.set_output("Buffer", arr);
    return true;
};
NODE_EXECUTION_FUNCTION(create_buffer2i)
{
    int val[2];
    for (int i = 0; i < 2; ++i) {
        val[i] = params.get_input<float>(socket_name(i).c_str());
    }
    auto s = params.get_input<int>("Size");
    pxr::GfVec2i data;
    memcpy(&data, val, sizeof(pxr::GfVec2i));
    pxr::VtArray<pxr::GfVec2i> arr;
    arr.resize(s, data);
    params.set_output("Buffer", arr);
    return true;
};
NODE_EXECUTION_FUNCTION(create_buffer3i)
{
    int val[3];
    for (int i = 0; i < 3; ++i) {
        val[i] = params.get_input<float>(socket_name(i).c_str());
    }
    auto s = params.get_input<int>("Size");
    pxr::GfVec3i data;
    memcpy(&data, val, sizeof(pxr::GfVec3i));
    pxr::VtArray<pxr::GfVec3i> arr;
    arr.resize(s, data);
    params.set_output("Buffer", arr);
    return true;
};
NODE_EXECUTION_FUNCTION(create_buffer4i)
{
    int val[4];
    for (int i = 0; i < 4; ++i) {
        val[i] = params.get_input<float>(socket_name(i).c_str());
    }
    auto s = params.get_input<int>("Size");
    pxr::GfVec4i data;
    memcpy(&data, val, sizeof(pxr::GfVec4i));
    pxr::VtArray<pxr::GfVec4i> arr;
    arr.resize(s, data);
    params.set_output("Buffer", arr);
    return true;
};

NODE_DECLARATION_UI(create_buffer);
NODE_DEF_CLOSE_SCOPE
