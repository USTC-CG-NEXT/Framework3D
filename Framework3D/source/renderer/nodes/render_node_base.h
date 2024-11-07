#pragma once
// #include "Nodes/node.hpp"
// #include "Nodes/node_exec.hpp"
// #include "Nodes/socket_types/render_socket_types.hpp"
// #include "Nodes/socket_types/stage_socket_types.hpp"
//
//
// #include "node_global_payload.h"
//
// USTC_CG_NAMESPACE_OPEN_SCOPE
// inline void render_node_type_base(NodeTypeInfo* ntype)
//{
//     ntype->color[0] = 114 / 255.f;
//     ntype->color[1] = 94 / 255.f;
//     ntype->color[2] = 29 / 255.f;
//     ntype->color[3] = 1.0f;
//
//     ntype->node_type_of_grpah = NodeTypeOfGrpah::Render;
// }

//
// USTC_CG_NAMESPACE_CLOSE_SCOPE
#include "../source/camera.h"
#include "../source/geometries/mesh.h"
#include "../source/light.h"
#include "../source/material.h"
#include "RHI/ResourceManager/resource_allocator.hpp"
#include "hd_USTC_CG/render_global_payload.hpp"
#include "nodes/core/node_exec.hpp"
#include "utils/resource_cleaner.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
inline Hd_USTC_CG_Camera* get_free_camera(
    ExeParams& params,
    const std::string& camera_name = "Camera")
{
    auto& cameras = params.get_global_payload<RenderGlobalPayload&>().get_cameras();

    Hd_USTC_CG_Camera* free_camera = nullptr;
    for (auto camera : cameras) {
        if (camera->GetId() != SdfPath::EmptyPath()) {
            free_camera = camera;
            break;
        }
    }
    return free_camera;
}

inline ResourceAllocator& get_resource_allocator(ExeParams& params)
{
    return params.get_global_payload<RenderGlobalPayload&>().resource_allocator;
}

#define resource_allocator (get_resource_allocator(params))

USTC_CG_NAMESPACE_CLOSE_SCOPE