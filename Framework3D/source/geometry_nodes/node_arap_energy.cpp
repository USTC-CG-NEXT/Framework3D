#include "GCore/Components/MeshOperand.h"
#include "GCore/util_openmesh_bind.h"
#include "geom_node_base.h"
#include <cmath>
#include <Eigen/Dense>

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(arap_energy)
{
    b.add_input<Eigen::MatrixXd>("SingularValues");
    b.add_input<Eigen::VectorXd>("Areas");

    b.add_output<float>("Energy");
}

NODE_EXECUTION_FUNCTION(arap_energy)
{
    // Get the input from params
    auto sigmas = params.get_input<Eigen::MatrixXd>("SingularValues");
    auto areas = params.get_input<Eigen::VectorXd>("Areas");

    if (areas.rows() != sigmas.rows()) {
        throw std::runtime_error("ARAP Energy: Wrong Input.");
    }

    double sum_area = 0;
    double energy = 0;

    for (int i = 0; i < sigmas.rows(); i++) {
        //std::cout << sigmas(i, 0) << "\t" << sigmas(i, 1) << std::endl;
        energy += areas(i) * (pow(sigmas(i, 0) - 1, 2) + pow(sigmas(i, 1) - 1, 2));
        sum_area += areas(i);
    }

    std::cout << float(energy / sum_area) << std::endl;
    
    params.set_output("Energy", float(energy / sum_area));
    return true;
}

NODE_DECLARATION_REQUIRED(arap_energy);

NODE_DECLARATION_UI(arap_energy);
NODE_DEF_CLOSE_SCOPE
