#include "glm/ext/matrix_float4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "nodes/core/def/node_def.hpp"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(transform_compose)
{
    b.add_input<float>("Translate X").min(-10).max(10).default_val(0);
    b.add_input<float>("Translate Y").min(-10).max(10).default_val(0);
    b.add_input<float>("Translate Z").min(-10).max(10).default_val(0);

    b.add_input<float>("Rotate X").min(-180).max(180).default_val(0);
    b.add_input<float>("Rotate Y").min(-180).max(180).default_val(0);
    b.add_input<float>("Rotate Z").min(-180).max(180).default_val(0);

    b.add_input<float>("Scale X").min(0.1f).max(10).default_val(1);
    b.add_input<float>("Scale Y").min(0.1f).max(10).default_val(1);
    b.add_input<float>("Scale Z").min(0.1f).max(10).default_val(1);

    b.add_output<glm::mat4x4>("Transform");
}

NODE_EXECUTION_FUNCTION(transform_compose)
{
    auto t_x = params.get_input<float>("Translate X");
    auto t_y = params.get_input<float>("Translate Y");
    auto t_z = params.get_input<float>("Translate Z");

    auto r_x = params.get_input<float>("Rotate X");
    auto r_y = params.get_input<float>("Rotate Y");
    auto r_z = params.get_input<float>("Rotate Z");

    auto s_x = params.get_input<float>("Scale X");
    auto s_y = params.get_input<float>("Scale Y");
    auto s_z = params.get_input<float>("Scale Z");

    glm::mat4x4 transform = glm::mat4x4(1.0);
    transform = glm::scale(transform, glm::vec3(s_x, s_y, s_z));
    transform = glm::rotate(transform, glm::radians(r_x), glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, glm::radians(r_y), glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, glm::radians(r_z), glm::vec3(0, 0, 1));
    transform = glm::translate(transform, glm::vec3(t_x, t_y, t_z));

    params.set_output("Transform", transform);
    return true;
}

NODE_DECLARATION_UI(transform_compose);
NODE_DEF_CLOSE_SCOPE