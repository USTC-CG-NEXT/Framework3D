#pragma once
#include "api.h"
#include "geometries/mesh.h"
#include "nvrhi/nvrhi.h"
#include "pxr/imaging/garch/glApi.h"
#include "pxr/imaging/hd/renderBuffer.h"
#include "pxr/pxr.h"

// SceneTypes
#include "../nodes/shaders/shaders/Scene/SceneTypes.slang"
#include "internal/memory/DeviceMemoryPool.hpp"
USTC_CG_NAMESPACE_OPEN_SCOPE

class HD_USTC_CG_API Hd_USTC_CG_RenderInstanceCollection {
   public:
    explicit Hd_USTC_CG_RenderInstanceCollection();
    ~Hd_USTC_CG_RenderInstanceCollection();

    nvrhi::rt::IAccelStruct *get_tlas();
    DescriptorTableManager *get_descriptor_table() const
    {
        return bindlessData.descriptorTableManager.get();
    }

    DeviceMemoryPool<unsigned> index_pool;
    DeviceMemoryPool<float> vertex_pool;
    DeviceMemoryPool<GeometryInstanceData> instance_pool;
    DeviceMemoryPool<nvrhi::rt::InstanceDesc> rt_instance_pool;
    DeviceMemoryPool<MeshDesc> mesh_pool;
    DeviceMemoryPool<nvrhi::DrawIndexedIndirectArguments> draw_indirect_pool;

    struct BindlessData {
        BindlessData();
        std::shared_ptr<DescriptorTableManager> descriptorTableManager;

        nvrhi::BindingLayoutHandle bindlessLayout;
    };
    BindlessData bindlessData;

    void set_require_rebuild_tlas()
    {
        require_rebuild_tlas = true;
    }

   private:
    nvrhi::rt::AccelStructHandle TLAS;

    bool require_rebuild_tlas = true;
    void rebuild_tlas();
};

USTC_CG_NAMESPACE_CLOSE_SCOPE