#include "nodes/core/def/node_def.hpp"
#include "pxr/imaging/hd/tokens.h"
#include "render_node_base.h"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(debug_info)
{
    b.add_input<entt::meta_any>("Variable");
}

NODE_EXECUTION_FUNCTION(debug_info)
{
    // Left empty.
    // auto lights = params.get_input<LightArray>("Lights");
    // auto cameras = params.get_input<CameraArray>("Camera");
    // auto meshes = params.get_input<MeshArray>("Meshes");
    // MaterialMap materials = params.get_input<MaterialMap>("Materials");

    auto& global_payload = params.get_global_payload<RenderGlobalPayload&>();

    for (auto&& camera : global_payload.get_cameras()) {
        std::cout << camera->GetTransform() << std::endl;
    }

    for (auto&& light : global_payload.get_lights()) {
        std::cout << light->Get(HdTokens->transform).Cast<GfMatrix4d>()
                  << std::endl;
    }

    for (auto&& mesh : global_payload.get_meshes()) {
        std::cout << mesh->GetId() << std::endl;
        auto names = mesh->GetBuiltinPrimvarNames();
        for (auto&& name : names) {
            std::cout << name.GetString() << std::endl;
        }
        log::info(
            "Mesh contained material: %s",
            global_payload.get_materials()[mesh->GetMaterialId()]
                ->GetId()
                .GetString()
                .c_str());
    }
    return true;
}

NODE_DECLARATION_UI(debug_info);
NODE_DEF_CLOSE_SCOPE
