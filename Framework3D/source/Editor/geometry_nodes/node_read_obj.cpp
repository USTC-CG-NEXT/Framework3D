
#include "nodes/core/def/node_def.hpp"
#include "igl/readOBJ.h"

NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(read_obj)
{
    b.add_input<std::string>("Path").default_val("Default");
    b.add_output<std::vector<std::vector<float>>>("Vertices");
    b.add_output<std::vector<std::vector<float>>>("Texture Coordinates");
    b.add_output<std::vector<std::vector<float>>>("Normals");
    b.add_output<std::vector<std::vector<int>>>("Faces");
    b.add_output<std::vector<std::vector<int>>>("Face Texture Coordinates");
    b.add_output<std::vector<std::vector<int>>>("Face Normals");
    // Function content omitted
}

NODE_EXECUTION_FUNCTION(read_obj)
{
    auto path = params.get_input<std::string>("Path");
    std::vector<std::vector<float>> V;
    std::vector<std::vector<float>> TC;
    std::vector<std::vector<float>> N;
    std::vector<std::vector<int>> F;
    std::vector<std::vector<int>> FTC;
    std::vector<std::vector<int>> FN;
    // Function content omitted
    auto success = igl::readOBJ(path, V, TC, N, F, FTC, FN);

    if (success) {
        params.set_output("Vertices",V);
        params.set_output("Texture Coordinates", TC);
        params.set_output("Normals", N);
        params.set_output("Faces", F);
        params.set_output("Face Texture Coordinates", FTC);
        params.set_output("Face Normals", FN);
        return true;
    }
    else {
        return false; 
    }
}

NODE_DECLARATION_REQUIRED(read_obj);
NODE_DECLARATION_UI(read_obj);
NODE_DEF_CLOSE_SCOPE
