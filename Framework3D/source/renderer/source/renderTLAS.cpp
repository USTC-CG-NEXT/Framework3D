#include "renderTLAS.h"

#include "RHI/rhi.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

Hd_USTC_CG_RenderInstanceCollection::Hd_USTC_CG_RenderInstanceCollection()
{
    nvrhi::rt::AccelStructDesc tlasDesc;
    tlasDesc.isTopLevel = true;
    tlasDesc.topLevelMaxInstances = 1024 * 1024;
    TLAS = RHI::get_device()->createAccelStruct(tlasDesc);
}

Hd_USTC_CG_RenderInstanceCollection::~Hd_USTC_CG_RenderInstanceCollection()
{
}

nvrhi::rt::IAccelStruct* Hd_USTC_CG_RenderInstanceCollection::get_tlas()
{
    if (rt_instance_pool.compress()) {
        require_rebuild_tlas = true;
    }
    if (require_rebuild_tlas) {
        rebuild_tlas();
        require_rebuild_tlas = false;
    }

    return TLAS;
}

Hd_USTC_CG_RenderInstanceCollection::BindlessData::BindlessData()
{
    auto device = RHI::get_device();
    nvrhi::BindlessLayoutDesc desc;
    desc.visibility = nvrhi::ShaderType::All;
    desc.maxCapacity = 8 * 1024;
    bindlessLayout = device->createBindlessLayout(desc);
    descriptorTableManager =
        std::make_unique<DescriptorTableManager>(device, bindlessLayout);
}

void Hd_USTC_CG_RenderInstanceCollection::rebuild_tlas()
{
    auto nvrhi_device = RHI::get_device();

    auto command_list = nvrhi_device->createCommandList();
    command_list->open();
    command_list->beginMarker("TLAS Update");
    command_list->buildTopLevelAccelStructFromBuffer(
        TLAS,
        rt_instance_pool.get_device_buffer(),
        0,
        rt_instance_pool.count());
    command_list->endMarker();
    command_list->close();
    nvrhi_device->executeCommandList(command_list);
    nvrhi_device->waitForIdle();
}

USTC_CG_NAMESPACE_CLOSE_SCOPE