
#include "camera.h"
#include "geometries/mesh.h"
#include "light.h"
#include "pxr/base/gf/frustum.h"
#include "pxr/imaging/glf/simpleLight.h"
#include "pxr/imaging/hd/tokens.h"
#include "render_node_base.h"
#include "resource_allocator_instance.hpp"
#include "rich_type_buffer.hpp"
#include "utils/draw_fullscreen.h"

#include "nodes/core/def/node_def.hpp"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(environment_pass)
{

    b.add_input<nvrhi::TextureHandle>("Color");
    b.add_input<nvrhi::TextureHandle>("Depth");


    b.add_input<std::string>("Shader").default_val("shaders/environment_map.fs");
    b.add_output<nvrhi::TextureHandle>("Color");
}

NODE_EXECUTION_FUNCTION(environment_pass)
{
#ifdef USTC_CG_BACKEND_OPENGL 
    auto lights = params.get_input<LightArray>("Lights");
    auto color = params.get_input<TextureHandle>("Color");

    Hd_USTC_CG_Dome_Light* dome_light = nullptr;
    Hd_USTC_CG_Camera* free_camera = get_free_camera(params);

    for (int i = 0; i < lights.size(); ++i) {
        auto light = lights[i];
        if (light->GetId() != SdfPath::EmptyPath()) {
            if (HdPrimTypeTokens->domeLight == light->GetLightType()) {
                dome_light = dynamic_cast<Hd_USTC_CG_Dome_Light*>(light);
                break;
            }
        }
    }

    // Search for the light with name first. If not found, fall back to anonymous lights.
    if (!dome_light) {
        for (int i = 0; i < lights.size(); ++i) {
            auto light = lights[i];
            if (HdPrimTypeTokens->domeLight == light->GetLightType()) {
                dome_light = dynamic_cast<Hd_USTC_CG_Dome_Light*>(light);
                break;
            }
        }
    }

    auto depth = params.get_input<TextureHandle>("Depth");

    auto size = color->desc.size;

    unsigned int VBO, VAO;

    CreateFullScreenVAO(VAO, VBO);

    TextureDesc texture_desc;
    texture_desc.size = size;
    texture_desc.format = HdFormatFloat32Vec4;
    auto color_texture = resource_allocator.create(texture_desc);

    auto shaderPath = params.get_input<std::string>("Shader");

    ShaderDesc shader_desc;
    shader_desc.set_vertex_path(
        std::filesystem::path(RENDER_NODES_FILES_DIR) /
        std::filesystem::path("shaders/fullscreen.vs"));

    shader_desc.set_fragment_path(
        std::filesystem::path(RENDER_NODES_FILES_DIR) / std::filesystem::path(shaderPath));
    auto shader_handle = resource_allocator.create(shader_desc);
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture->texture_id, 0);

    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    shader_handle->shader.use();
    shader_handle->shader.setVec2("iResolution", size);

    unsigned id = 0;
    if (dome_light) {
        dome_light->RefreshGLBuffer();
        dome_light->BindTextures(shader_handle->shader, id);
    }

    shader_handle->shader.setInt("color", id);
    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_2D, color->texture_id);
    id++;

    shader_handle->shader.setInt("depth", id);
    glActiveTexture(GL_TEXTURE0 + id);
    glBindTexture(GL_TEXTURE_2D, depth->texture_id);
    id++;
    shader_handle->shader.setMat4("view", GfMatrix4f(free_camera->viewMatrix));
    shader_handle->shader.setMat4("projection", GfMatrix4f(free_camera->projMatrix));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    DestroyFullScreenVAO(VAO, VBO);

    auto shader_error = shader_handle->shader.get_error();

    resource_allocator.destroy(shader_handle);

    params.set_output("Color", color_texture);

    if (!shader_error.empty()) {
        throw std::runtime_error(shader_error);
    }
#endif

}


NODE_DECLARATION_UI(environment_pass);
NODE_DEF_CLOSE_SCOPE
