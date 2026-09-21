#pragma once

// Owns the Vulkan objects that live for the whole run: instance, debug
// messenger, surface, physical device, logical device, queue, and the VMA
// allocator.
//
// volk provides the function pointers, so VK_NO_PROTOTYPES must be defined
// project-wide (it comes from the volk CMake target) and volk.h must be
// included before anything else that includes vulkan.h.

#include <volk.h>

#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

struct GLFWwindow;

class VulkanContext
{
public:
    VulkanContext() = default;

    ~VulkanContext()
    {
        destroy();
    }

    // VulkanContext should be created once and passed by reference as needed,
    // so we should disable copies/moves
    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;
    VulkanContext(VulkanContext&&) = delete;
    VulkanContext& operator=(VulkanContext&&) = delete;

    bool init(GLFWwindow* window, bool enableValidation = true);
    void destroy();

    bool supportsSER() const
    {
        return m_serSupported;
    }

    VkInstance getInstance() const
    {
        return m_vkbInstance.instance;
    }

    VkPhysicalDevice getPhysicalDevice() const
    {
        return m_vkbDevice.physical_device;
    }

    VkDevice getDevice() const
    {
        return m_vkbDevice.device;
    }

    VkQueue getGraphicsQueue() const
    {
        return m_graphicsQueue;
    }

    uint32_t getGraphicsQueueFamily() const
    {
        return m_graphicsQueueFamily;
    }

    VmaAllocator getAllocator() const
    {
        return m_allocator;
    }

    // Expose the vk-boostrap device object for Swapchain creation.
    // In addition to the logical raw VkDevice, vkb::Device holds the physical device, the surface, and the
    // queue family indices, all of which swapchain creation needs.
    const vkb::Device& getVkbDevice() const
    {
        return m_vkbDevice;
    }

    // Physical device RT properties hold driver-defined limits, like shader binding table (SBT) alignment.
    const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& getRtProperties() const
    {
        return m_rtProperties;
    }

private:
    vkb::Instance m_vkbInstance{};
    vkb::Device m_vkbDevice{};

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    uint32_t m_graphicsQueueFamily = 0;
    VmaAllocator m_allocator = VK_NULL_HANDLE;

    VkPhysicalDeviceProperties m_deviceProperties{};
    VkPhysicalDeviceDriverProperties m_driverProperties{};
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_rtProperties{};
    VkRayTracingInvocationReorderModeNV m_serReorderMode = VK_RAY_TRACING_INVOCATION_REORDER_MODE_NONE_NV;

    bool m_serSupported = false;

    bool initInstance(bool enableValidation);
    bool initSurface(GLFWwindow* window);
    bool initPhysicalDevice(vkb::PhysicalDevice& physicalDevice);
    bool initLogicalDevice(const vkb::PhysicalDevice& physicalDevice);
    bool initVmaAllocator();
    bool initDeviceProperties();
    void printDeviceReport() const;
};
