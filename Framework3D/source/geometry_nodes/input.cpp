
#include "nodes/core/def/node_def.hpp"
#include <Eigen/Eigen>

NODE_DEF_OPEN_SCOPE       
NODE_DECLARATION_FUNCTION(input)
{
    // Function content omitted
    b.add_input<float>("X").min(-10).max(10).default_val(0);
    b.add_input<float>("Y").min(-10).max(10).default_val(0);
    b.add_input<float>("Z").min(-10).max(10).default_val(0);
    b.add_output<Eigen::MatrixXd>("Output");
}

NODE_EXECUTION_FUNCTION(input)
{
    // Function content omitted
    Eigen::MatrixXd Output(1, 3);
    Output(0, 0) = static_cast<double> (params.get_input<float>("X"));
    Output(0, 1) = static_cast<double> (params.get_input<float>("Y"));
    Output(0, 2) = static_cast<double> (params.get_input<float>("Z"));
    params.set_output("Output", Output);
    return true;
}

NODE_DECLARATION_UI(input);
NODE_DEF_CLOSE_SCOPE
