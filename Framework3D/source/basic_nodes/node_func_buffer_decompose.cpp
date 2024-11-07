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

NODE_DECLARATION_FUNCTION(decompose_buffer2f)
{
    b.add_input<float2Buffer>("Buffer");
    for (int i = 0; i < 2; ++i) {
        b.add_output<float1Buffer>(socket_name(i).c_str());
    }
    b.add_output<int>("Size");
};
NODE_DECLARATION_FUNCTION(decompose_buffer3f)
{
    b.add_input<float3Buffer>("Buffer");
    for (int i = 0; i < 3; ++i) {
        b.add_output<float1Buffer>(socket_name(i).c_str());
    }
    b.add_output<int>("Size");
};
NODE_DECLARATION_FUNCTION(decompose_buffer4f)
{
    b.add_input<float4Buffer>("Buffer");
    for (int i = 0; i < 4; ++i) {
        b.add_output<float1Buffer>(socket_name(i).c_str());
    }
    b.add_output<int>("Size");
};

NODE_DECLARATION_FUNCTION(decompose_buffer2i)
{
    b.add_input<int2Buffer>("Buffer");
    for (int i = 0; i < 2; ++i) {
        b.add_output<int1Buffer>(socket_name(i).c_str());
    }
    b.add_output<int>("Size");
};
NODE_DECLARATION_FUNCTION(decompose_buffer3i)
{
    b.add_input<int3Buffer>("Buffer");
    for (int i = 0; i < 3; ++i) {
        b.add_output<int1Buffer>(socket_name(i).c_str());
    }
    b.add_output<int>("Size");
};
NODE_DECLARATION_FUNCTION(decompose_buffer4i)
{
    b.add_input<int4Buffer>("Buffer");
    for (int i = 0; i < 4; ++i) {
        b.add_output<int1Buffer>(socket_name(i).c_str());
    }
    b.add_output<int>("Size");
};

NODE_EXECUTION_FUNCTION(decompose_buffer2f)
{
    pxr::VtArray<float> val[2];
    auto input = params.get_input<pxr::VtArray<pxr::GfVec2f>>("Buffer");
    for (int i = 0; i < 2; ++i) {
        val[i].resize(input.size());
    }
    for (int idx = 0; idx < input.size(); ++idx) {
        for (int i = 0; i < 2; ++i) {
            val[i][idx] = input[idx][i];
        }
    }
    for (int i = 0; i < 2; ++i) {
        params.set_output(socket_name(i).c_str(), val[i]);
    }
    params.set_output("Size", int(input.size()));
    return true;
};
;
NODE_EXECUTION_FUNCTION(decompose_buffer3f)
{
    pxr::VtArray<float> val[3];
    auto input = params.get_input<pxr::VtArray<pxr::GfVec3f>>("Buffer");
    for (int i = 0; i < 3; ++i) {
        val[i].resize(input.size());
    }
    for (int idx = 0; idx < input.size(); ++idx) {
        for (int i = 0; i < 3; ++i) {
            val[i][idx] = input[idx][i];
        }
    }
    for (int i = 0; i < 3; ++i) {
        params.set_output(socket_name(i).c_str(), val[i]);
    }
    params.set_output("Size", int(input.size()));
    return true;
};
;
NODE_EXECUTION_FUNCTION(decompose_buffer4f)
{
    pxr::VtArray<float> val[4];
    auto input = params.get_input<pxr::VtArray<pxr::GfVec4f>>("Buffer");
    for (int i = 0; i < 4; ++i) {
        val[i].resize(input.size());
    }
    for (int idx = 0; idx < input.size(); ++idx) {
        for (int i = 0; i < 4; ++i) {
            val[i][idx] = input[idx][i];
        }
    }
    for (int i = 0; i < 4; ++i) {
        params.set_output(socket_name(i).c_str(), val[i]);
    }
    params.set_output("Size", int(input.size()));
    return true;
};
;

NODE_EXECUTION_FUNCTION(decompose_buffer2i)
{
    pxr::VtArray<int> val[2];
    auto input = params.get_input<pxr::VtArray<pxr::GfVec2i>>("Buffer");
    for (int i = 0; i < 2; ++i) {
        val[i].resize(input.size());
    }
    for (int idx = 0; idx < input.size(); ++idx) {
        for (int i = 0; i < 2; ++i) {
            val[i][idx] = input[idx][i];
        }
    }
    for (int i = 0; i < 2; ++i) {
        params.set_output(socket_name(i).c_str(), val[i]);
    }
    params.set_output("Size", int(input.size()));
    return true;
};
;
NODE_EXECUTION_FUNCTION(decompose_buffer3i)
{
    pxr::VtArray<int> val[3];
    auto input = params.get_input<pxr::VtArray<pxr::GfVec3i>>("Buffer");
    for (int i = 0; i < 3; ++i) {
        val[i].resize(input.size());
    }
    for (int idx = 0; idx < input.size(); ++idx) {
        for (int i = 0; i < 3; ++i) {
            val[i][idx] = input[idx][i];
        }
    }
    for (int i = 0; i < 3; ++i) {
        params.set_output(socket_name(i).c_str(), val[i]);
    }
    params.set_output("Size", int(input.size()));
    return true;
};
;
NODE_EXECUTION_FUNCTION(decompose_buffer4i)
{
    pxr::VtArray<int> val[4];
    auto input = params.get_input<pxr::VtArray<pxr::GfVec4i>>("Buffer");
    for (int i = 0; i < 4; ++i) {
        val[i].resize(input.size());
    }
    for (int idx = 0; idx < input.size(); ++idx) {
        for (int i = 0; i < 4; ++i) {
            val[i][idx] = input[idx][i];
        }
    }
    for (int i = 0; i < 4; ++i) {
        params.set_output(socket_name(i).c_str(), val[i]);
    }
    params.set_output("Size", int(input.size()));
    return true;
};
;

NODE_DECLARATION_UI(buffer_decompose);
NODE_DEF_CLOSE_SCOPE
