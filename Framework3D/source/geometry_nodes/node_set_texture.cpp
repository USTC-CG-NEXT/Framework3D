#include "GCore/Components/MaterialComponent.h"
#include "geom_node_base.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(declare)
{
    b.add_input<Geometry>("Geometry");
    b.add_input<std::string>("Texture Name").default_val("");
    b.add_output<Geometry>("Geometry");
}

NODE_EXECUTION_FUNCTION(exec)
{
    auto texture = params.get_input<std::string>("Texture Name");

    auto geometry = params.get_input<Geometry>("Geometry");
    auto material = geometry.get_component<MaterialComponent>();
    if (!material) {
        material = std::make_shared<MaterialComponent>(&geometry);
    }
    material->textures.clear();
    material->textures.push_back(texture);
    geometry.attach_component(material);

    params.set_output("Geometry", std::move(geometry));
}



NODE_DECLARATION_UI(set_texture);
NODE_DEF_CLOSE_SCOPE
