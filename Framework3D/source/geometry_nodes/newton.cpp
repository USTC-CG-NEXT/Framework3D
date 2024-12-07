#include <Eigen/Eigen>
#include "nodes/core/def/node_def.hpp"

NODE_DEF_OPEN_SCOPE
using Vec = Eigen::Vector3f;

NODE_DECLARATION_FUNCTION(newton)
{
    b.add_input<Vec>("Input");
    b.add_output<Vec>("Output");

}

NODE_EXECUTION_FUNCTION(newton)
{
    auto input = params.get_input<Vec>("Input");

    Vec result = { 0, 0, 0 };





    params.set_output<Vec>("Output", std::move(result));

}

NODE_DECLARATION_UI(newton);
NODE_DEF_CLOSE_SCOPE