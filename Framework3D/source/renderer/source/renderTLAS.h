#pragma once
#include "api.h"
#include "geometries/mesh.h"
#include "nvrhi/nvrhi.h"
#include "pxr/imaging/garch/glApi.h"
#include "pxr/imaging/hd/renderBuffer.h"
#include "pxr/pxr.h"

USTC_CG_NAMESPACE_OPEN_SCOPE

class Hd_USTC_CG_RenderTLAS {
   public:
    explicit Hd_USTC_CG_RenderTLAS();
    ~Hd_USTC_CG_RenderTLAS();

    nvrhi::rt::AccelStructHandle get_tlas();

    nvrhi::IBuffer* update_model_transforms();

    std::vector<nvrhi::rt::InstanceDesc> &acquire_instances_to_edit(
        HdRprim *mesh);
    void removeInstance(HdRprim *hd_ustc_cg_mesh);

    std::mutex edit_instances_mutex;

   private:
    void rebuild_tlas();

    nvrhi::rt::AccelStructHandle TLAS;
    std::map<HdRprim *, std::vector<nvrhi::rt::InstanceDesc>> instances;
    bool require_rebuild_tlas = true;
};

USTC_CG_NAMESPACE_CLOSE_SCOPE