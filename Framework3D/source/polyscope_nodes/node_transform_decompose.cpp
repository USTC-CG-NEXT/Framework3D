#include "glm/ext/matrix_float4x4.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "nodes/core/def/node_def.hpp"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(transform_decompose)
{
    b.add_input<glm::mat4x4>("Transform");

    b.add_output<float>("Translate X");
    b.add_output<float>("Translate Y");
    b.add_output<float>("Translate Z");

    b.add_output<float>("Rotate X");
    b.add_output<float>("Rotate Y");
    b.add_output<float>("Rotate Z");

    b.add_output<float>("Scale X");
    b.add_output<float>("Scale Y");
    b.add_output<float>("Scale Z");
}

NODE_EXECUTION_FUNCTION(transform_decompose)
{
    auto transform = params.get_input<glm::mat4x4>("Transform");

    // Extract translation
    auto translation = transform[3];
    params.set_output("Translate X", translation[0]);
    params.set_output("Translate Y", translation[1]);
    params.set_output("Translate Z", translation[2]);

    // Extract rotation
    auto rotation = glm::eulerAngles(glm::quat_cast(transform));
    params.set_output("Rotate X", glm::degrees(rotation[0]));
    params.set_output("Rotate Y", glm::degrees(rotation[1]));
    params.set_output("Rotate Z", glm::degrees(rotation[2]));

    // Extract scale
    auto scale = glm::vec3(
        glm::length(glm::vec3(transform[0])),
        glm::length(glm::vec3(transform[1])),
        glm::length(glm::vec3(transform[2])));
    params.set_output("Scale X", scale[0]);
    params.set_output("Scale Y", scale[1]);
    params.set_output("Scale Z", scale[2]);

    return true;
}

NODE_DECLARATION_UI(transform_decompose);
NODE_DEF_CLOSE_SCOPE
