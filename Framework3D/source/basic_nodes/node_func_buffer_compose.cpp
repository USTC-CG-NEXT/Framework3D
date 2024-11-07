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

NODE_DECLARATION_FUNCTION(compose_buffer2f)
{
    b.add_output<float2Buffer>("Buffer");
    for (int i = 0; i < 2; ++i) {
        b.add_input<float1Buffer>(socket_name(i).c_str());
    }
};
NODE_DECLARATION_FUNCTION(compose_buffer3f)
{
    b.add_output<float3Buffer>("Buffer");
    for (int i = 0; i < 3; ++i) {
        b.add_input<float1Buffer>(socket_name(i).c_str());
    }
};
NODE_DECLARATION_FUNCTION(compose_buffer4f)
{
    b.add_output<float4Buffer>("Buffer");
    for (int i = 0; i < 4; ++i) {
        b.add_input<float1Buffer>(socket_name(i).c_str());
    }
};

NODE_DECLARATION_FUNCTION(compose_buffer2i)
{
    b.add_output<int2Buffer>("Buffer");
    for (int i = 0; i < 2; ++i) {
        b.add_input<int1Buffer>(socket_name(i).c_str());
    }
};
NODE_DECLARATION_FUNCTION(compose_buffer3i)
{
    b.add_output<int3Buffer>("Buffer");
    for (int i = 0; i < 3; ++i) {
        b.add_input<int1Buffer>(socket_name(i).c_str());
    }
};
NODE_DECLARATION_FUNCTION(compose_buffer4i)
{
    b.add_output<int4Buffer>("Buffer");
    for (int i = 0; i < 4; ++i) {
        b.add_input<int1Buffer>(socket_name(i).c_str());
    }
};

NODE_EXECUTION_FUNCTION(compose_buffer2f)
{
    pxr::VtArray<pxr::GfVec2f> Buffer;
    pxr::VtArray<float> val[2];
    for (int i = 0; i < 2; ++i) {
        val[i] = params.get_input<pxr::VtArray<float>>(socket_name(i).c_str());
    }
    size_t max_size = 0;
    for (int i = 0; i < 2; ++i) {
        max_size = std::max(val[i].size(), max_size);
    }
    Buffer.resize(max_size);
    for (int idx = 0; idx < max_size; ++idx) {
        for (int i = 0; i < 2; ++i) {
            if (idx < val[i].size()) {
                Buffer[idx][i] = val[i][idx];
            }
            else {
                Buffer[idx][i] = 0;
            }
        }
    }
    params.set_output("Buffer", Buffer);
    return true;
};
;
NODE_EXECUTION_FUNCTION(compose_buffer3f)
{
    pxr::VtArray<pxr::GfVec3f> Buffer;
    pxr::VtArray<float> val[3];
    for (int i = 0; i < 3; ++i) {
        val[i] = params.get_input<pxr::VtArray<float>>(socket_name(i).c_str());
    }
    size_t max_size = 0;
    for (int i = 0; i < 3; ++i) {
        max_size = std::max(val[i].size(), max_size);
    }
    Buffer.resize(max_size);
    for (int idx = 0; idx < max_size; ++idx) {
        for (int i = 0; i < 3; ++i) {
            if (idx < val[i].size()) {
                Buffer[idx][i] = val[i][idx];
            }
            else {
                Buffer[idx][i] = 0;
            }
        }
    }
    params.set_output("Buffer", Buffer);
    return true;
};
;
NODE_EXECUTION_FUNCTION(compose_buffer4f)
{
    pxr::VtArray<pxr::GfVec4f> Buffer;
    pxr::VtArray<float> val[4];
    for (int i = 0; i < 4; ++i) {
        val[i] = params.get_input<pxr::VtArray<float>>(socket_name(i).c_str());
    }
    size_t max_size = 0;
    for (int i = 0; i < 4; ++i) {
        max_size = std::max(val[i].size(), max_size);
    }
    Buffer.resize(max_size);
    for (int idx = 0; idx < max_size; ++idx) {
        for (int i = 0; i < 4; ++i) {
            if (idx < val[i].size()) {
                Buffer[idx][i] = val[i][idx];
            }
            else {
                Buffer[idx][i] = 0;
            }
        }
    }
    params.set_output("Buffer", Buffer);
    return true;
};
;

NODE_EXECUTION_FUNCTION(compose_buffer2i)
{
    pxr::VtArray<pxr::GfVec2i> Buffer;
    pxr::VtArray<int> val[2];
    for (int i = 0; i < 2; ++i) {
        val[i] = params.get_input<pxr::VtArray<int>>(socket_name(i).c_str());
    }
    size_t max_size = 0;
    for (int i = 0; i < 2; ++i) {
        max_size = std::max(val[i].size(), max_size);
    }
    Buffer.resize(max_size);
    for (int idx = 0; idx < max_size; ++idx) {
        for (int i = 0; i < 2; ++i) {
            if (idx < val[i].size()) {
                Buffer[idx][i] = val[i][idx];
            }
            else {
                Buffer[idx][i] = 0;
            }
        }
    }
    params.set_output("Buffer", Buffer);
    return true;
};
;
NODE_EXECUTION_FUNCTION(compose_buffer3i)
{
    pxr::VtArray<pxr::GfVec3i> Buffer;
    pxr::VtArray<int> val[3];
    for (int i = 0; i < 3; ++i) {
        val[i] = params.get_input<pxr::VtArray<int>>(socket_name(i).c_str());
    }
    size_t max_size = 0;
    for (int i = 0; i < 3; ++i) {
        max_size = std::max(val[i].size(), max_size);
    }
    Buffer.resize(max_size);
    for (int idx = 0; idx < max_size; ++idx) {
        for (int i = 0; i < 3; ++i) {
            if (idx < val[i].size()) {
                Buffer[idx][i] = val[i][idx];
            }
            else {
                Buffer[idx][i] = 0;
            }
        }
    }
    params.set_output("Buffer", Buffer);
    return true;
};
;
NODE_EXECUTION_FUNCTION(compose_buffer4i)
{
    pxr::VtArray<pxr::GfVec4i> Buffer;
    pxr::VtArray<int> val[4];
    for (int i = 0; i < 4; ++i) {
        val[i] = params.get_input<pxr::VtArray<int>>(socket_name(i).c_str());
    }
    size_t max_size = 0;
    for (int i = 0; i < 4; ++i) {
        max_size = std::max(val[i].size(), max_size);
    }
    Buffer.resize(max_size);
    for (int idx = 0; idx < max_size; ++idx) {
        for (int i = 0; i < 4; ++i) {
            if (idx < val[i].size()) {
                Buffer[idx][i] = val[i][idx];
            }
            else {
                Buffer[idx][i] = 0;
            }
        }
    }
    params.set_output("Buffer", Buffer);
    return true;
};
;

NODE_DECLARATION_UI(buffer_compose);
NODE_DEF_CLOSE_SCOPE
