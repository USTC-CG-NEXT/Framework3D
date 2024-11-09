#include "renderTLAS.h"

#include "RHI/rhi.hpp"

USTC_CG::Hd_USTC_CG_RenderTLAS::Hd_USTC_CG_RenderTLAS()
{
    nvrhi::rt::AccelStructDesc tlasDesc;
    tlasDesc.isTopLevel = true;
    tlasDesc.topLevelMaxInstances = 114514;
    TLAS = RHI::get_device()->createAccelStruct(tlasDesc);
}

USTC_CG::Hd_USTC_CG_RenderTLAS::~Hd_USTC_CG_RenderTLAS()
{
}

nvrhi::rt::AccelStructHandle USTC_CG::Hd_USTC_CG_RenderTLAS::get_tlas()
{
    if (require_rebuild_tlas) {
        rebuild_tlas();
        require_rebuild_tlas = false;
    }

    return TLAS;
}

void USTC_CG::Hd_USTC_CG_RenderTLAS::removeInstance(HdRprim* rPrim)
{
    std::lock_guard lock(edit_instances_mutex);
    if (instances.contains(rPrim)) {
        instances.erase(rPrim);
    }
    require_rebuild_tlas = true;
}

std::vector<nvrhi::rt::InstanceDesc>&
USTC_CG::Hd_USTC_CG_RenderTLAS::acquire_instances_to_edit(HdRprim* mesh)
{
    require_rebuild_tlas = true;
    return instances[mesh];
}

void USTC_CG::Hd_USTC_CG_RenderTLAS::rebuild_tlas()
{
    std::vector<nvrhi::rt::InstanceDesc> instances_vec;

    // Iterate through the map and concatenate all vectors
    for (const auto& pair : instances) {
        const std::vector<nvrhi::rt::InstanceDesc>& vec = pair.second;
        instances_vec.insert(instances_vec.end(), vec.begin(), vec.end());
    }
    auto nvrhi_device = RHI::get_device();

    auto command_list = nvrhi_device->createCommandList();

    command_list->open();
    command_list->beginMarker("TLAS Update");
    command_list->buildTopLevelAccelStruct(
        TLAS, instances_vec.data(), instances_vec.size());
    command_list->endMarker();

    command_list->close();

    nvrhi_device->executeCommandList(command_list);

    nvrhi_device->waitForIdle();
}