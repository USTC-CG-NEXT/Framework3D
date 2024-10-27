#include <nvrhi/nvrhi.h>

#include <RHI/rhi.hpp>

#include "nvrhi/vulkan.h"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include <vulkan/vulkan.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

USTC_CG_NAMESPACE_OPEN_SCOPE
nvrhi::DeviceHandle device;
int init()
{
    if (!device) {
        nvrhi::vulkan::DeviceDesc desc;

        // Initialize the required fields
        vk::DynamicLoader dl;
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
            dl.getProcAddress<PFN_vkGetInstanceProcAddr>(
                "vkGetInstanceProcAddr");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

        vk::ApplicationInfo appInfo(
            "USTC_CG", 1, "USTC_CG_RHI", 1, VK_API_VERSION_1_3);
        vk::InstanceCreateInfo instanceCreateInfo({}, &appInfo);
        vk::Instance instance = vk::createInstance(instanceCreateInfo);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

        std::vector<vk::PhysicalDevice> physicalDevices =
            instance.enumeratePhysicalDevices();
        vk::PhysicalDevice physicalDevice = physicalDevices[0];

        float queuePriority = 1.0f;
        vk::DeviceQueueCreateInfo queueCreateInfo({}, 0, 1, &queuePriority);
        vk::DeviceCreateInfo deviceCreateInfo({}, queueCreateInfo);

        // Check if ray tracing is supported
        vk::PhysicalDeviceRayTracingPipelineFeaturesKHR
            rayTracingPipelineFeatures;
        vk::PhysicalDeviceAccelerationStructureFeaturesKHR
            accelerationStructureFeatures;
        vk::PhysicalDeviceBufferDeviceAddressFeatures
            bufferDeviceAddressFeatures;

        vk::PhysicalDeviceFeatures2 deviceFeatures2;
        deviceFeatures2.pNext = &rayTracingPipelineFeatures;
        rayTracingPipelineFeatures.pNext = &accelerationStructureFeatures;
        accelerationStructureFeatures.pNext = &bufferDeviceAddressFeatures;

        physicalDevice.getFeatures2(&deviceFeatures2);

        std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
            VK_KHR_SPIRV_1_4_EXTENSION_NAME,
            VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME
        };

        if (rayTracingPipelineFeatures.rayTracingPipeline &&
            accelerationStructureFeatures.accelerationStructure) {
            deviceExtensions.push_back(
                VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
            deviceExtensions.push_back(
                VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
            deviceExtensions.push_back(
                VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        }

        deviceCreateInfo.setPNext(&deviceFeatures2);
        deviceCreateInfo.setPEnabledExtensionNames(deviceExtensions);

        vk::Device vk_device = physicalDevice.createDevice(deviceCreateInfo);
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vk_device);

        desc.instance = instance;
        desc.physicalDevice = physicalDevice;
        desc.device = vk_device;

        // Optionally initialize other fields if needed
        vk::Queue graphicsQueue = vk_device.getQueue(0, 0);
        vk::Queue transferQueue = vk_device.getQueue(0, 0);
        vk::Queue computeQueue = vk_device.getQueue(0, 0);

        desc.graphicsQueue = graphicsQueue;
        desc.graphicsQueueIndex = 0;
        desc.transferQueue = transferQueue;
        desc.transferQueueIndex = 0;
        desc.computeQueue = computeQueue;
        desc.computeQueueIndex = 0;
        desc.allocationCallbacks = nullptr;

        const char* instanceExtensions[] = { "VK_KHR_surface",
                                             "VK_KHR_win32_surface" };
        desc.instanceExtensions = instanceExtensions;
        desc.numInstanceExtensions =
            sizeof(instanceExtensions) / sizeof(instanceExtensions[0]);

        desc.deviceExtensions = deviceExtensions.data();
        desc.numDeviceExtensions =
            static_cast<uint32_t>(deviceExtensions.size());

        desc.maxTimerQueries = 256;
        desc.bufferDeviceAddressSupported = true;

        device = createDevice(desc);
    }
    return device != nullptr;
}

nvrhi::DeviceHandle get_device()
{
    return device;
}

int shutdown()
{
    device = nullptr;
    return device == nullptr;
}

USTC_CG_NAMESPACE_CLOSE_SCOPE