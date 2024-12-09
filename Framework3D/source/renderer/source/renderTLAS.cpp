#include "renderTLAS.h"

#include "RHI/rhi.hpp"

USTC_CG::Hd_USTC_CG_RenderInstanceCollection::
    Hd_USTC_CG_RenderInstanceCollection()
{
    nvrhi::rt::AccelStructDesc tlasDesc;
    tlasDesc.isTopLevel = true;
    tlasDesc.topLevelMaxInstances = 1024 * 1024;
    TLAS = RHI::get_device()->createAccelStruct(tlasDesc);
}

USTC_CG::Hd_USTC_CG_RenderInstanceCollection::
    ~Hd_USTC_CG_RenderInstanceCollection()
{
}

nvrhi::rt::IAccelStruct*
USTC_CG::Hd_USTC_CG_RenderInstanceCollection::get_tlas()
{
    if (require_rebuild_tlas) {
        rebuild_tlas();
        require_rebuild_tlas = false;
    }

    return TLAS;
}

void USTC_CG::Hd_USTC_CG_RenderInstanceCollection::removeInstance(
    HdRprim* rPrim)
{
    if (instances.contains(rPrim)) {
        instances.erase(rPrim);
    }
    require_rebuild_tlas = true;
}

USTC_CG::Hd_USTC_CG_RenderInstanceCollection::BindlessData::BindlessData()
{
    auto device = RHI::get_device();
    nvrhi::BindlessLayoutDesc desc;
    desc.visibility = nvrhi::ShaderType::All;
    desc.maxCapacity = 8 * 1024;
    bindlessLayout = device->createBindlessLayout(desc);
    descriptorTableManager =
        std::make_unique<DescriptorTableManager>(device, bindlessLayout);
}

std::vector<USTC_CG::InstanceDescription>&
USTC_CG::Hd_USTC_CG_RenderInstanceCollection::acquire_instances_to_edit(
    HdRprim* mesh)
{
    require_rebuild_tlas = true;
    return instances[mesh];
}

void USTC_CG::Hd_USTC_CG_RenderInstanceCollection::rebuild_tlas()
{
    std::vector<nvrhi::rt::InstanceDesc> instances_vec;

    // Iterate through the map and concatenate all vectors
    for (const auto& pair : instances) {
        const std::vector<InstanceDescription>& vec = pair.second;
        for (const auto& instance : vec) {
            instances_vec.push_back(instance.rt_instance_desc);
        }
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