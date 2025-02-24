#include "renderer.h"

#include "camera.h"
#include "node_exec_eager_render.hpp"
#include "pxr/imaging/hd/renderBuffer.h"
#include "pxr/imaging/hd/tokens.h"
#include "renderBuffer.h"
#include "renderParam.h"

USTC_CG_NAMESPACE_OPEN_SCOPE
using namespace pxr;

Hd_USTC_CG_Renderer::Hd_USTC_CG_Renderer(Hd_USTC_CG_RenderParam* render_param)
    : _enableSceneColors(false),
      render_param(render_param)
{
}

Hd_USTC_CG_Renderer::~Hd_USTC_CG_Renderer()
{
    auto executor = dynamic_cast<EagerNodeTreeExecutorRender*>(
        render_param->node_system->get_node_tree_executor());
    executor->reset_allocator();
}

void Hd_USTC_CG_Renderer::Render(HdRenderThread* renderThread)
{
    _completedSamples.store(0);

    render_param->presented_texture = nullptr;
    auto node_system = render_param->node_system;

    {
        auto& global_payload = node_system->get_node_tree_executor()
                                   ->get_global_payload<RenderGlobalPayload&>();

        global_payload.resource_allocator.gc();

        global_payload.InstanceCollection =
            render_param->InstanceCollection.get();
        global_payload.lens_system = render_param->lens_system;

        global_payload.reset_accumulation = false;

        node_system->execute(false);
    }
    //RHI::get_device()->waitForIdle();
    //RHI::get_device()->runGarbageCollection();

    for (size_t i = 0; i < _aovBindings.size(); ++i) {
        std::string present_name;
        nvrhi::TextureHandle texture = nullptr;

        if (_aovBindings[i].aovName == HdAovTokens->depth) {
            present_name = "present_depth";
        }

        if (_aovBindings[i].aovName == HdAovTokens->color) {
            present_name = "present_color";
        }

        for (auto&& node : node_system->get_node_tree()->nodes) {
            auto try_fetch_info = [&node, node_system]<typename T>(
                                      const char* id_name, T& obj) {
                if (std::string(node->typeinfo->id_name) == id_name) {
                    assert(node->get_inputs().size() == 1);
                    auto output_socket = node->get_inputs()[0];
                    entt::meta_any data;
                    node_system->get_node_tree_executor()
                        ->sync_node_to_external_storage(output_socket, data);
                    obj = data.cast<T>();
                }
            };
            try_fetch_info(present_name.c_str(), texture);
            if (texture) {
                break;
            }
        }
        if (texture) {
            auto rb = static_cast<Hd_USTC_CG_RenderBuffer*>(
                _aovBindings[i].renderBuffer);
#ifdef USTC_CG_DIRECT_VK_DISPLAY
            render_param->presented_texture = texture;
#else
            rb->Present(texture);
#endif
            rb->SetConverged(true);
        }
    }

    node_system->finalize();

    // executor->finalize(node_tree);
}

void Hd_USTC_CG_Renderer::Clear()
{
    for (size_t i = 0; i < _aovBindings.size(); ++i) {
        if (_aovBindings[i].clearValue.IsEmpty()) {
            continue;
        }

        auto rb =
            static_cast<Hd_USTC_CG_RenderBuffer*>(_aovBindings[i].renderBuffer);
        rb->Clear();
    }
}

void Hd_USTC_CG_Renderer::SetAovBindings(
    const HdRenderPassAovBindingVector& aovBindings)
{
    _aovBindings = aovBindings;
    _aovNames.resize(_aovBindings.size());
    for (size_t i = 0; i < _aovBindings.size(); ++i) {
        _aovNames[i] = HdParsedAovToken(_aovBindings[i].aovName);
    }

    // Re-validate the attachments.
    _aovBindingsNeedValidation = true;
}

void Hd_USTC_CG_Renderer::MarkAovBuffersUnconverged()
{
    for (size_t i = 0; i < _aovBindings.size(); ++i) {
        auto rb =
            static_cast<Hd_USTC_CG_RenderBuffer*>(_aovBindings[i].renderBuffer);
        rb->SetConverged(false);
    }
}

void Hd_USTC_CG_Renderer::renderTimeUpdateCamera(
    const HdRenderPassStateSharedPtr& renderPassState)
{
    camera_ =
        static_cast<const Hd_USTC_CG_Camera*>(renderPassState->GetCamera());
    camera_->update(renderPassState);
}

bool Hd_USTC_CG_Renderer::nodetree_modified()
{
    //    return render_param->node_tree->GetDirty();
    return false;
}

bool Hd_USTC_CG_Renderer::nodetree_modified(bool new_status)
{
    // auto old_status = render_param->node_tree->GetDirty();
    // render_param->node_tree->SetDirty(new_status);
    // return old_status;

    return false;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
