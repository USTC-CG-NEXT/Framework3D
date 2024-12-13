#include "renderTLAS.h"

#include "RHI/rhi.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE

Hd_USTC_CG_RenderInstanceCollection::Hd_USTC_CG_RenderInstanceCollection()
    : rt_instance_pool(BufferDesc{}.setIsAccelStructBuildInput(true)),
      index_pool(
          BufferDesc{}.setIsIndexBuffer(true).setIsAccelStructBuildInput(true)),
      vertex_pool(
          BufferDesc{}.setIsVertexBuffer(true).setIsAccelStructBuildInput(
              true)),
      draw_indirect_pool(BufferDesc{}.setIsDrawIndirectArgs(true))
{
    nvrhi::rt::AccelStructDesc tlasDesc;
    tlasDesc.isTopLevel = true;
    tlasDesc.topLevelMaxInstances = 1024 * 1024;
    TLAS = RHI::get_device()->createAccelStruct(tlasDesc);

    vertex_pool.reserve(64 * 1024 * 3);
    index_pool.reserve(64 * 1024);
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
    desc.addRegisterSpace(nvrhi::BindingLayoutItem::StructuredBuffer_SRV(1));
    desc.addRegisterSpace(nvrhi::BindingLayoutItem::Texture_SRV(2));
    bindlessLayout = device->createBindlessLayout(desc);
    descriptorTableManager =
        std::make_shared<DescriptorTableManager>(device, bindlessLayout);
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
}

USTC_CG_NAMESPACE_CLOSE_SCOPE