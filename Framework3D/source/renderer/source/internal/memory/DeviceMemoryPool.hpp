#pragma once
#include <RHI/rhi.hpp>
#include <algorithm>
#include <unordered_map>

#include "../../api.h"
USTC_CG_NAMESPACE_OPEN_SCOPE

template<typename T>
class DeviceMemoryPool {
   public:
    struct MemoryHandleData {
        static std::shared_ptr<MemoryHandleData> create();
        size_t offset = INVALID;
        size_t size = 0;

        void write_data(void* data);

        ~MemoryHandleData();

        nvrhi::BindingSetItem get_descriptor(
            nvrhi::ResourceType type =
                nvrhi::ResourceType::StructuredBuffer_UAV)
        {
            nvrhi::BindingSetItem item;
            item.resourceHandle = pool->device_buffer;
            item.range = nvrhi::BufferRange{
                .offset = offset,
                .size = size,
            };
            item.type = type;
            return item;
        }

       private:
        static constexpr size_t INVALID = -1;
        DeviceMemoryPool* pool;
        friend class DeviceMemoryPool;
    };

    using MemoryHandle = std::shared_ptr<MemoryHandleData>;

    DeviceMemoryPool();
    void clear();
    void destroy();
    ~DeviceMemoryPool();

    void gc();
    void reserve(size_t size);
    MemoryHandle allocate(size_t count);

    nvrhi::IBuffer* get_device_buffer() const;
    size_t max_memory_offset() const;
    size_t pool_size() const;
    size_t size() const;

    std::string info(bool free_list = true) const;

    // For debugging
    bool sanitize();

   private:
    void relocate_buffer();
    void erase(MemoryHandleData* handle);

    // Assume the command list (of the other one) is already opened
    void adopt(MemoryHandleData* handle_from_another_pool);
    std::vector<MemoryHandleData*> handles_allocated;

    nvrhi::BufferHandle device_buffer;
    std::vector<std::pair<size_t, size_t>> h_free_list;  // offset, size
    nvrhi::BufferHandle compact_buffer_cache;
    size_t max_count = 1;
    size_t targeted_max_count = 1;
    size_t current_count = 0;
    size_t current_max_memory_offset = 0;
    nvrhi::CommandListHandle commandList;

    // Utility functions
    template<typename U>
    nvrhi::BufferDesc buffer_desc()
    {
        nvrhi::BufferDesc desc;
        desc.structStride = sizeof(U);
        desc.byteSize = max_count * sizeof(U);
        desc.setCanHaveUAVs(true);
        desc.keepInitialState = true;
        return desc;
    }
};

template<typename T>
void DeviceMemoryPool<T>::reserve(size_t size)
{
    while (size > targeted_max_count) {
        targeted_max_count *= 2;
    }

    relocate_buffer();
}

template<typename T>
std::shared_ptr<typename DeviceMemoryPool<T>::MemoryHandleData>
DeviceMemoryPool<T>::MemoryHandleData::create()
{
    return std::make_shared<MemoryHandleData>();
}

template<typename T>
void DeviceMemoryPool<T>::MemoryHandleData::write_data(void* data)
{
    auto device = pool->get_device_buffer();
    pool->commandList->open();
    pool->commandList->writeBuffer(device, data, size, offset);
    pool->commandList->close();
    RHI::get_device()->executeCommandList(pool->commandList);
    RHI::get_device()->runGarbageCollection();
}

template<typename T>
DeviceMemoryPool<T>::MemoryHandleData::~MemoryHandleData()
{
    pool->erase(this);
}

template<typename T>
DeviceMemoryPool<T>::DeviceMemoryPool()
{
    RHI::init();
    nvrhi::IDevice* device = RHI::get_device();
    commandList = device->createCommandList();

    // Initialize device buffer and valid mask
    nvrhi::BufferDesc bufferDesc = buffer_desc<T>();
    bufferDesc.debugName = "DeviceObjectPoolBuffer";
    device_buffer = device->createBuffer(bufferDesc);

    nvrhi::BufferDesc maskDesc = buffer_desc<uint8_t>();
    maskDesc.debugName = "DeviceObjectPoolValidMask";
}

template<typename T>
void DeviceMemoryPool<T>::destroy()
{
    clear();
    commandList = nullptr;
    device_buffer = nullptr;
    compact_buffer_cache = nullptr;
}

template<typename T>
DeviceMemoryPool<T>::~DeviceMemoryPool()
{
    destroy();
}

template<typename T>
typename DeviceMemoryPool<T>::MemoryHandle DeviceMemoryPool<T>::allocate(
    size_t count)
{
    auto size = count * sizeof(T);
    MemoryHandle handle = MemoryHandleData::create();
    handle->size = size;
    handle->pool = this;

    for (auto free_handle = h_free_list.begin();
         free_handle != h_free_list.end();
         ++free_handle) {
        if (std::get<1>(*free_handle) >= size) {
            handle->offset = std::get<0>(*free_handle);

            if (std::get<1>(*free_handle) > size) {
                std::get<0>(*free_handle) += size;
                std::get<1>(*free_handle) -= size;
            }
            else {
                h_free_list.erase(free_handle);
            }
            break;
        }
    }

    if (handle->offset == MemoryHandleData::INVALID) {
        handle->offset = current_max_memory_offset;
        current_max_memory_offset += size;
    }

    handles_allocated.push_back(handle.get());
    current_count += count;

    while (targeted_max_count < current_max_memory_offset / sizeof(T)) {
        targeted_max_count *= 2;
    }

    relocate_buffer();

    return handle;
}

template<typename T>
void DeviceMemoryPool<T>::erase(MemoryHandleData* handle)
{
    h_free_list.push_back(std::make_pair(handle->offset, handle->size));
    current_count -= handle->size / sizeof(T);
    handles_allocated.erase(
        std::remove(handles_allocated.begin(), handles_allocated.end(), handle),
        handles_allocated.end());
}

template<typename T>
void DeviceMemoryPool<T>::adopt(MemoryHandleData* handle_from_another_pool)
{
    auto old_pool = handle_from_another_pool->pool;

    old_pool->commandList->copyBuffer(  // Copy data from another pool
        device_buffer,
        current_max_memory_offset,
        handle_from_another_pool->pool->get_device_buffer(),
        handle_from_another_pool->offset,
        handle_from_another_pool->size);

    current_count += handle_from_another_pool->size / sizeof(T);
    handles_allocated.push_back(handle_from_another_pool);
    handle_from_another_pool->offset = current_max_memory_offset;
    current_max_memory_offset += handle_from_another_pool->size;
}

template<typename T>
void DeviceMemoryPool<T>::clear()
{
    h_free_list.clear();
    current_count = 0;
    current_max_memory_offset = 0;
}

template<typename T>
void DeviceMemoryPool<T>::gc()
{
    // Create another DeviceMemoryPool and copy into it...
    DeviceMemoryPool<T> new_pool;
    new_pool.reserve(max_count);

    commandList->open();
    for (auto handle : handles_allocated) {
        new_pool.adopt(handle);
    }
    commandList->close();
    auto device = RHI::get_device();
    device->executeCommandList(commandList);
    device->runGarbageCollection();

    *this = std::move(new_pool);
}

template<typename T>
std::string DeviceMemoryPool<T>::info(bool free_list) const
{
    std::stringstream ss;

    ss << "[size]: " << current_count << std::endl;
    ss << "[max size]: " << max_count << std::endl;
    ss << "[max memory offset]: " << current_max_memory_offset << std::endl;

    if (free_list) {
        ss << "[Free list]: " << std::endl;
        for (int i = 0; i < h_free_list.size(); ++i) {
            ss << "  " << i << ": " << h_free_list[i].first / sizeof(T) << " - "
               << (h_free_list[i].first + h_free_list[i].second) / sizeof(T)
               << std::endl;
        }
    }
    return ss.str();
}

template<typename T>
bool DeviceMemoryPool<T>::sanitize()
{
    size_t current_offset = 0;
    std::sort(
        handles_allocated.begin(),
        handles_allocated.end(),
        [](MemoryHandleData* a, MemoryHandleData* b) {
            return a->offset < b->offset;
        });

    for (auto handle : handles_allocated) {
        if (handle->offset != current_offset) {
            return false;
        }
        current_offset += handle->size;
    }
    return true;
}

template<typename T>
nvrhi::IBuffer* DeviceMemoryPool<T>::get_device_buffer() const
{
    return device_buffer;
}

template<typename T>
size_t DeviceMemoryPool<T>::max_memory_offset() const
{
    return current_max_memory_offset;
}

template<typename T>
size_t DeviceMemoryPool<T>::pool_size() const
{
    return max_count;
}

template<typename T>
size_t DeviceMemoryPool<T>::size() const
{
    return current_count;
}

template<typename T>
void DeviceMemoryPool<T>::relocate_buffer()
{
    if (max_count != targeted_max_count) {
        nvrhi::BufferDesc bufferDesc = buffer_desc<T>();
        bufferDesc.byteSize = targeted_max_count * sizeof(T);
        bufferDesc.debugName = "DeviceObjectPoolBuffer";
        auto new_device_buffer = RHI::get_device()->createBuffer(bufferDesc);

        commandList->open();
        commandList->copyBuffer(
            new_device_buffer, 0, device_buffer, 0, max_count * sizeof(T));

        commandList->close();

        RHI::get_device()->executeCommandList(commandList);
        RHI::get_device()->runGarbageCollection();

        max_count = targeted_max_count;

        device_buffer = new_device_buffer;
    }
}

USTC_CG_NAMESPACE_CLOSE_SCOPE