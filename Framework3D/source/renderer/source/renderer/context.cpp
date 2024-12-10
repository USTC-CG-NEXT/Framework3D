

#include "context.hpp"

USTC_CG_NAMESPACE_OPEN_SCOPE
GPUContext::~GPUContext()
{
    resource_allocator_.destroy(commandList_);
}

GPUContext::GPUContext(ResourceAllocator& resource_allocator, ProgramVars& vars)
    : resource_allocator_(resource_allocator),
      vars_(vars)
{
    commandList_ = resource_allocator_.create(CommandListDesc{});
}

void GPUContext::begin()
{
    commandList_->open();
}

void GPUContext::finish()
{
    commandList_->close();
    resource_allocator_.device->executeCommandList(commandList_);
}

void GPUContext::clear_buffer(
    nvrhi::IBuffer* buffer,
    uint32_t clear_value,
    const nvrhi::BufferRange& range)
{
    commandList_->clearBufferUInt(buffer, clear_value);
}

void GPUContext::clear_texture(
    nvrhi::ITexture* texture,
    nvrhi::Color color,
    const nvrhi::TextureSubresourceSet& subresources)
{
    commandList_->clearTextureFloat(texture, subresources, color);
}

USTC_CG_NAMESPACE_CLOSE_SCOPE
