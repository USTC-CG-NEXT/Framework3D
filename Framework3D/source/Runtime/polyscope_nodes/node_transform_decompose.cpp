#include "glm/ext/matrix_float4x4.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/matrix_decompose.hpp"
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

    glm::vec3 scale, translation, skew;
    glm::vec4 perspective;
    glm::quat rotation;

    bool success = glm::decompose(
        transform, scale, rotation, translation, skew, perspective);
    if (!success) {
        return false;
    }

    // Extract translation
    params.set_output("Translate X", translation.x);
    params.set_output("Translate Y", translation.y);
    params.set_output("Translate Z", translation.z);

    // Extract rotation
    auto eulerAngles = glm::eulerAngles(rotation);
    params.set_output("Rotate X", glm::degrees(eulerAngles.x));
    params.set_output("Rotate Y", glm::degrees(eulerAngles.y));
    params.set_output("Rotate Z", glm::degrees(eulerAngles.z));

    // Extract scale
    params.set_output("Scale X", scale.x);
    params.set_output("Scale Y", scale.y);
    params.set_output("Scale Z", scale.z);

    return true;
}

NODE_DECLARATION_UI(transform_decompose);
NODE_DEF_CLOSE_SCOPE
