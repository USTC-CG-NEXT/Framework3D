

#include "nodes/core/def/node_def.hpp"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/imaging/hd/tokens.h"
#include "pxr/imaging/hgiGL/computeCmds.h"
#include "render_node_base.h"
#include "rich_type_buffer.hpp"
NODE_DEF_OPEN_SCOPE
NODE_DECLARATION_FUNCTION(select)
{
    b.add_input<std::string>("Vertex Shader")
        .default_val("shaders/select.vs");
    b.add_input<std::string>("Fragment Shader")
        .default_val("shaders/select.fs");
    b.add_output<TextureHandle>("ColorID");
}

NODE_EXECUTION_FUNCTION(select)
{
    try {
        Hd_USTC_CG_Camera* free_camera = get_free_camera(params);

        auto size = free_camera->_dataWindow.GetSize();

        TextureDesc texture_desc;
        texture_desc.size = size;
        texture_desc.format = HdFormatInt32;    // 4 Byte
        auto colorID_texture = resource_allocator.create(texture_desc);

        auto vs_path = params.get_input<std::string>("Vertex Shader");
        auto fs_path = params.get_input<std::string>("Fragment Shader");

        ShaderDesc shader_desc;
        shader_desc.set_vertex_path(
            std::filesystem::path(RENDER_NODES_FILES_DIR) /
            std::filesystem::path(vs_path));

        shader_desc.set_fragment_path(
            std::filesystem::path(RENDER_NODES_FILES_DIR) /
            std::filesystem::path(fs_path));

        auto shader_handle = resource_allocator.create(shader_desc);

        glBindTexture(GL_TEXTURE_2D, colorID_texture->texture_id);
        /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);  */
        /*glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_R32I,
            size[0],
            size[1],
            0,
            GL_RED_INTEGER,
            GL_INT,
            nullptr);*/

        //glDisable(GL_BLEND);
        //glDisable(GL_MULTISAMPLE);
        //glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        GLuint framebuffer;
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            colorID_texture->texture_id,
            0);

        /*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);*/

        GLenum attachments[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, attachments);

        //glClearColor(1.f, 0.f, 0.f, 0.0f);
        GLint clearValue[] = { 1000 };
        glClearBufferiv(GL_COLOR, 0, clearValue);
        glClear(
            GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, size[0], size[1]);

        shader_handle->shader.use();
        shader_handle->shader.setMat4(
            "view", GfMatrix4f(free_camera->_viewMatrix));
        shader_handle->shader.setMat4(
            "projection", GfMatrix4f(free_camera->_projMatrix));

        for (int i = 0; i < meshes.size(); ++i) {
            auto mesh = meshes[i];
            shader_handle->shader.setMat4("model", mesh->transform);
            shader_handle->shader.setInt("pID", i);

            mesh->RefreshGLBuffer();

            glBindVertexArray(mesh->VAO);
            //glPointSize(5.0f);
            //glEnable(GL_POINT_SMOOTH);
            glDrawElements(
                GL_TRIANGLES,
                static_cast<unsigned int>(mesh->triangulatedIndices.size() * 3),
                GL_UNSIGNED_INT,
                0);
            glBindVertexArray(0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &framebuffer);

        auto shader_error = shader_handle->shader.get_error();

        params.set_output("ColorID", colorID_texture);
        
        std::vector<int32_t> pixels(size[0] * size[1]);

        glBindTexture(GL_TEXTURE_2D, colorID_texture->texture_id);
        glGetTexImage(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            GL_INT,  // 数据类型必须匹配纹理
            pixels.data());

        int count = 0;
        for (int i = 0; i < size[0] * size[1]; ++i) {
            if (pixels[i] != 0) {
                count++;
            }
        }
        if (GLenum err = glGetError(); err != GL_NO_ERROR) {
            std::cerr << "glGetTexImage failed (Error: " << err << ")"
                      << std::endl;
        }
        resource_allocator.destroy(shader_handle);

        std::cout << "Selected points count: " << count << std::endl;

        if (!shader_error.empty()) {
            throw std::runtime_error(shader_error);
        }
    }
    catch (std::exception e) {
        return false;
    }
}

NODE_DECLARATION_UI(select);
NODE_DEF_CLOSE_SCOPE
