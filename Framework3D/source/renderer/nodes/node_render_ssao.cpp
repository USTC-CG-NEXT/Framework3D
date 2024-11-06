
#include "camera.h"
#include "light.h"
#include "pxr/imaging/hd/tokens.h"
#include "render_node_base.h"
#include "resource_allocator_instance.hpp"
#include "rich_type_buffer.hpp"
#include "utils/draw_fullscreen.h"

#include "nodes/core/def/node_def.hpp"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(ssao)
{
    b.add_input<nvrhi::TextureHandle>("Color");
    b.add_input<nvrhi::TextureHandle>("Position");
    b.add_input<nvrhi::TextureHandle>("Depth");

    // HW6: For HBAO you might need normal texture.

    b.add_input<std::string>("Shader").default_val("shaders/ssao.fs");
    b.add_output<nvrhi::TextureHandle>("Color");
}

NODE_EXECUTION_FUNCTION(ssao)
{
#ifdef USTC_CG_BACKEND_OPENGL  // Temporarily only enable opengl. Later it can be refactored to
                               // support nvrhi
    auto color = params.get_input<TextureHandle>("Color");

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
    auto shader = resource_allocator.create(shader_desc);
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(
        GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture->texture_id, 0);

    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    shader->shader.use();
    shader->shader.setVec2("iResolution", size);

    // HW6: Bind the textures like other passes here.

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    DestroyFullScreenVAO(VAO, VBO);
    resource_allocator.destroy(shader);

    params.set_output("Color", color_texture);
#endif
}



NODE_DECLARATION_UI(ssao);
NODE_DEF_CLOSE_SCOPE
