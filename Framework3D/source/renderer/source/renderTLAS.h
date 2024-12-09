#pragma once
#include "api.h"
#include "geometries/mesh.h"
#include "nvrhi/nvrhi.h"
#include "pxr/imaging/garch/glApi.h"
#include "pxr/imaging/hd/renderBuffer.h"
#include "pxr/pxr.h"

// SceneTypes
#include "../nodes/shaders/shaders/Scene/SceneTypes.slang"

USTC_CG_NAMESPACE_OPEN_SCOPE

struct InstanceDescription {
    nvrhi::rt::InstanceDesc rt_instance_desc;
    GeometryInstanceData geometry_instance_data;
};

class HD_USTC_CG_API Hd_USTC_CG_RenderInstanceCollection {
   public:
    explicit Hd_USTC_CG_RenderInstanceCollection();
    ~Hd_USTC_CG_RenderInstanceCollection();

    nvrhi::rt::IAccelStruct *get_tlas();
    DescriptorTableManager *get_descriptor_table() const
    {
        return bindlessData.descriptorTableManager.get();
    }

    std::vector<InstanceDescription> &acquire_instances_to_edit(HdRprim *mesh);
    void removeInstance(HdRprim *hd_ustc_cg_mesh);

    std::mutex edit_instances_mutex;

   private:
    struct BindlessData {
        BindlessData();
        std::unique_ptr<DescriptorTableManager> descriptorTableManager;

       private:
        nvrhi::BindingLayoutHandle bindlessLayout;
    };
    void rebuild_tlas();
    BindlessData bindlessData;

    nvrhi::rt::AccelStructHandle TLAS;
    std::map<HdRprim *, std::vector<InstanceDescription>> instances;
    bool require_rebuild_tlas = true;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE