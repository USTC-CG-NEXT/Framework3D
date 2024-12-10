#pragma once
#include <RHI/rhi.hpp>
#include <unordered_map>

#include "../../api.h"
USTC_CG_NAMESPACE_OPEN_SCOPE
template<typename T>
class DeviceObjectPool {
   public:
    // This handle would not be invalidated even if the pool is compacted.
    // (Pointing to the new target)
    struct DeviceObjectHandle {
        bool isValid()
        {
            return index != INVALID && pool->version == this->version;
        }
        static constexpr size_t INVALID = -1;
        size_t index = INVALID;
        DeviceObjectPool* pool;
        unsigned version;
    };

    DeviceObjectPool();
    ~DeviceObjectPool() = default;
    DeviceObjectHandle insert(const T& data, bool gc_immediately = true);
    void erase(DeviceObjectHandle& handle);
    void clear();

    // replace the device_buffer with the compact buffer, and if a compaction is
    // performed, update the handles
    void gc();

    nvrhi::IBuffer* get_device_buffer() const
    {
        return device_buffer;
    }

    nvrhi::IBuffer* get_valid_mask() const
    {
        return d_valid_mask;
    }

    nvrhi::IBuffer* compact_buffer(bool with_mapping_info = false) const;

    size_t size() const
    {
        return current_size;
    }

    void update_handle(DeviceObjectHandle& handle);
    // Since this could be possibly updated, the handle should be updated
    unsigned get_object_location(DeviceObjectHandle& handle)
    {
        update_handle();
    }

   private:
    void relocate_buffer();

    nvrhi::BufferHandle device_buffer;

    // Indicates which part of the device buffer is released and once again
    // ready to be re-written
    std::vector<size_t> h_free_list;

    // If one object is erased, it's not valid anymore, but not immediately
    // removed from the device buffer. Instead, it's added to the free list and
    // the valid mask is updated.
    nvrhi::BufferHandle d_valid_mask;

    // Sometimes, we want the buffer to be contiguous, so we need to compact it
    nvrhi::BufferHandle compact_buffer_cache;

    // When the buffer is full, it's reallocated with double the size
    size_t max_size = 16;
    size_t current_size = 0;
    size_t current_max_index = 0;

    unsigned version = 0;

    std::vector<std::unordered_map<size_t, size_t>> updates;

    nvrhi::CommandListHandle commandList;
};

template<typename T>
DeviceObjectPool<T>::DeviceObjectPool()
{
    nvrhi::IDevice* device = RHI::get_device();
    commandList = device->createCommandList();
    
}

template<typename T>
typename DeviceObjectPool<T>::DeviceObjectHandle DeviceObjectPool<T>::insert(
    const T& data,
    bool gc_immediately)
{
    current_size++;
    if (current_size > max_size) {
        relocate_buffer();
    }

    auto device = RHI::get_device();
}

template<typename T>
void DeviceObjectPool<T>::erase(DeviceObjectHandle& handle)
{
}

// Only set the valid mask and free_list
template<typename T>
void DeviceObjectPool<T>::clear()
{
}

template<typename T>
void DeviceObjectPool<T>::gc()
{
}

template<typename T>
nvrhi::IBuffer* DeviceObjectPool<T>::compact_buffer(
    bool with_mapping_info) const
{
}

template<typename T>
void DeviceObjectPool<T>::update_handle(DeviceObjectHandle& handle)
{
    if (handle.version < version) {
        handle.version = version;
        auto& update_map = updates[version];
        handle.index = update_map[handle.index];
    }
}

template<typename T>
void DeviceObjectPool<T>::relocate_buffer()
{
}

USTC_CG_NAMESPACE_CLOSE_SCOPE