#include <memory>

#include "nodes/core/def/node_def.hpp"
#include "polyscope/polyscope.h"
#include "polyscope/surface_mesh.h"
#include "polyscope_widget/polyscope_renderer.h"
#include "polyscope_widget/polyscope_widget_payload.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

NODE_DEF_OPEN_SCOPE

NODE_DECLARATION_FUNCTION(set_image)
{
    b.add_input<std::string>("Image Path").default_val("");
}

NODE_EXECUTION_FUNCTION(set_image)
{
    auto path = params.get_input<std::string>("Image Path");

    int width, height, channels;
    unsigned char* data =
        stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "failed to load image from " << path << std::endl;
        return false;
    }

    bool has_alpha = (channels == 4);
    // Parse the data in to a float array
    std::vector<std::array<float, 3>> image_color(width * height);
    std::vector<std::array<float, 4>> image_color_alpha(width * height);
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            int pix_ind = (j * width + i) * channels;
            unsigned char p_r = data[pix_ind + 0];
            unsigned char p_g = data[pix_ind + 1];
            unsigned char p_b = data[pix_ind + 2];
            unsigned char p_a = 255;
            if (channels == 4) {
                p_a = data[pix_ind + 3];
            }

            // color
            std::array<float, 3> val{ p_r / 255.f, p_g / 255.f, p_b / 255.f };
            image_color[j * width + i] = val;

            // color alpha
            std::array<float, 4> val_a{
                p_r / 255.f, p_g / 255.f, p_b / 255.f, p_a / 255.f
            };
            image_color_alpha[j * width + i] = val_a;
        }
    }

    // Initialize a 2d face on XY plane
    std::vector<std::array<double, 2>> vertices = {
        { 0, 0 },
        { 1, 0 },
        { 1, 1 },
        { 0, 1 },
    };
    std::vector<std::vector<size_t>> faceVertexIndicesNested = {
        { 0, 1, 2, 3 },
    };
    auto surface_mesh_2d = polyscope::registerSurfaceMesh2D(
        "image", vertices, faceVertexIndicesNested);

    // Add vertex parameterization
    std::vector<std::array<float, 2>> vertex_parameterization = {
        { 0, 0 },
        { 1, 0 },
        { 1, 1 },
        { 0, 1 },
    };
    surface_mesh_2d->addVertexParameterizationQuantity(
        "vertex parameterization", vertex_parameterization);

    // Add image color quantity
    if (has_alpha) {
        surface_mesh_2d
            ->addTextureColorQuantity(
                "image color alpha",
                "vertex parameterization",
                width,
                height,
                image_color_alpha,
                polyscope::ImageOrigin::UpperLeft)
            ->setEnabled(true);
    }
    else {
        surface_mesh_2d
            ->addTextureColorQuantity(
                "image color",
                "vertex parameterization",
                width,
                height,
                image_color,
                polyscope::ImageOrigin::UpperLeft)
            ->setEnabled(true);
    }

    stbi_image_free(data);

    return true;
}

NODE_DECLARATION_UI(set_image);
NODE_DEF_CLOSE_SCOPE