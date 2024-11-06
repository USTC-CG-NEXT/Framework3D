#include "camera.h"
#include "geometries/mesh.h"
#include "light.h"
#include "material.h"
#include "pxr/imaging/hd/tokens.h"
#include "render_node_base.h"
#include "rich_type_buffer.hpp"

#include "nodes/core/def/node_def.hpp"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(debug_info)
{





    b.add_input<entt::meta_any>("Variable");
}

NODE_EXECUTION_FUNCTION(debug_info)
{
    // Left empty.
    auto lights = params.get_input<LightArray>("Lights");
    auto cameras = params.get_input<CameraArray>("Camera");
    auto meshes = params.get_input<MeshArray>("Meshes");
    MaterialMap materials = params.get_input<MaterialMap>("Materials");

    for (auto&& camera : cameras) {
        std::cout << camera->GetTransform() << std::endl;
    }

    for (auto&& light : lights) {
        std::cout << light->Get(HdTokens->transform).Cast<GfMatrix4d>()
                  << std::endl;
    }

    for (auto&& mesh : meshes) {
        std::cout << mesh->GetId() << std::endl;
        auto names = mesh->GetBuiltinPrimvarNames();
        for (auto&& name : names) {
            std::cout << name.GetString() << std::endl;
        }
        logging(
            "Mesh contained material: " +
                materials[mesh->GetMaterialId()]->GetId().GetString(),
            Info);
    }

}



NODE_DECLARATION_UI(debug_info);
NODE_DEF_CLOSE_SCOPE
