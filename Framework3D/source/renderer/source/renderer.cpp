#include "renderer.h"

#include "camera.h"
#include "nvrhi/d3d12.h"
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
    // auto executor =
    //     dynamic_cast<EagerNodeTreeExecutorRender*>(render_param->executor);
    // executor->reset_allocator();
}

void Hd_USTC_CG_Renderer::Render(HdRenderThread* renderThread)
{
    _completedSamples.store(0);

    auto node_system = render_param->node_system;

    node_system->execute();

    for (size_t i = 0; i < _aovBindings.size(); ++i) {
        std::string present_name = "render_present";
        nvrhi::TextureHandle texture = nullptr;

        if (_aovBindings[i].aovName == HdAovTokens->depth) {
            present_name = "render_present_depth";
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
            rb->Present(texture);
            rb->SetConverged(true);
        }
    }

    RHI::get_device()->runGarbageCollection();

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

        rb->Map();

        if (_aovNames[i].name == HdAovTokens->color) {
            GfVec4f clearColor = _GetClearColor(_aovBindings[i].clearValue);

            rb->Clear(clearColor.data());
        }
        else if (rb->GetFormat() == HdFormatInt32) {
            int32_t clearValue = _aovBindings[i].clearValue.Get<int32_t>();
            rb->Clear(&clearValue);
        }
        else if (rb->GetFormat() == HdFormatFloat32) {
            float clearValue = _aovBindings[i].clearValue.Get<float>();
            rb->Clear(&clearValue);
        }
        else if (rb->GetFormat() == HdFormatFloat32Vec3) {
            auto clearValue = _aovBindings[i].clearValue.Get<GfVec3f>();
            rb->Clear(clearValue.data());

        }  // else, _ValidateAovBindings would have already warned.

        rb->Unmap();
        rb->SetConverged(false);
    }
}

/* static */
GfVec4f Hd_USTC_CG_Renderer::_GetClearColor(const VtValue& clearValue)
{
    HdTupleType type = HdGetValueTupleType(clearValue);
    if (type.count != 1) {
        return GfVec4f(0.0f, 0.0f, 0.0f, 1.0f);
    }

    switch (type.type) {
        case HdTypeFloatVec3: {
            GfVec3f f =
                *(static_cast<const GfVec3f*>(HdGetValueData(clearValue)));
            return GfVec4f(f[0], f[1], f[2], 1.0f);
        }
        case HdTypeFloatVec4: {
            GfVec4f f =
                *(static_cast<const GfVec4f*>(HdGetValueData(clearValue)));
            return f;
        }
        case HdTypeDoubleVec3: {
            GfVec3d f =
                *(static_cast<const GfVec3d*>(HdGetValueData(clearValue)));
            return GfVec4f(f[0], f[1], f[2], 1.0f);
        }
        case HdTypeDoubleVec4: {
            GfVec4d f =
                *(static_cast<const GfVec4d*>(HdGetValueData(clearValue)));
            return GfVec4f(f);
        }
        default: return GfVec4f(0.0f, 0.0f, 0.0f, 1.0f);
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
