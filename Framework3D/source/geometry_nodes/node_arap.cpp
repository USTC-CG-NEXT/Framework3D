#include "GCore/Components/MeshOperand.h"
#include "GCore/util_openmesh_bind.h"
#include "geom_node_base.h"

/*
** @brief HW5_ARAP_Parameterization
**
** This file presents the basic framework of a "node", which processes inputs
** received from the left and outputs specific variables for downstream nodes to
** use.
**
** - In the first function, node_declare, you can set up the node's input and
** output variables.
**
** - The second function, node_exec is the execution part of the node, where we
** need to implement the node's functionality.
**
** - The third function generates the node's registration information, which
** eventually allows placing this node in the GUI interface.
**
** Your task is to fill in the required logic at the specified locations
** within this template, especially in node_exec.
*/

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(arap)
{
    // Input-1: Original 3D mesh with boundary
    // Maybe you need to add another input for initialization?
    b.add_input<Geometry>("Input");

    /*
    ** NOTE: You can add more inputs or outputs if necessary. For example, in
    ** some cases, additional information (e.g. other mesh geometry, other
    ** parameters) is required to perform the computation.
    **
    ** Be sure that the input/outputs do not share the same name. You can add
    ** one geometry as
    **
    **                b.add_input<Geometry>("Input");
    **
    ** Or maybe you need a value buffer like:
    **
    **                b.add_input<float1Buffer>("Weights");
    */

    // Output-1: The UV coordinate of the mesh, provided by ARAP algorithm
    b.add_output<pxr::GfVec2f>("OutputUV");
}

NODE_EXECUTION_FUNCTION(arap)
{
    // Get the input from params
    auto input = params.get_input<Geometry>("Input");

    // Avoid processing the node when there is no input
    if (!input.get_component<MeshComponent>()) {
        throw std::runtime_error("Need Geometry Input.");
    }
    throw std::runtime_error("Not implemented");

    /* ----------------------------- Preprocess -------------------------------
    ** Create a halfedge structure (using OpenMesh) for the input mesh. The
    ** half-edge data structure is a widely used data structure in geometric
    ** processing, offering convenient operations for traversing and modifying
    ** mesh elements.
    */
    auto halfedge_mesh = operand_to_openmesh(&input);

    /* ------------- [HW5_TODO] ARAP Parameterization Implementation -----------
    ** Implement ARAP mesh parameterization to minimize local distortion.
    **
    ** Steps:
    ** 1. Initial Setup: Use a HW4 parameterization result as initial setup.
    **
    ** 2. Local Phase: For each triangle, compute local orthogonal approximation
    **    (Lt) by computing SVD of Jacobian(Jt) with fixed u.
    **
    ** 3. Global Phase: With Lt fixed, update parameter coordinates(u) by
    *solving
    **    a pre-factored global sparse linear system.
    **
    ** 4. Iteration: Repeat Steps 2 and 3 to refine parameterization.
    **
    ** Note:
    **  - Fixed points' selection is crucial for ARAP and ASAP.
    **  - Encapsulate algorithms into classes for modularity.
    */

    // The result UV coordinates
    pxr::VtArray<pxr::GfVec2f> uv_result;

    // Set the output of the node
    params.set_output("OutputUV", uv_result);
}

NODE_DECLARATION_UI(arap);
NODE_DEF_CLOSE_SCOPE
