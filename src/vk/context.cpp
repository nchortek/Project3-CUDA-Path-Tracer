#include "context.h"

#include <GLFW/glfw3.h>

#include <cstdio>

bool VulkanContext::init(GLFWwindow* window, bool enableValidation)
{
    (void)window;
    (void)enableValidation;

    // 1. volkInitialize()
    //
    // 2. vkb::InstanceBuilder: app name, require_api_version(1, 3, 0),
    //    request_validation_layers(enableValidation), use_default_debug_messenger().
    //    GLFW's required instance extensions are added by vk-bootstrap.
    //    → volkLoadInstance(m_instance)
    //
    // 3. glfwCreateWindowSurface(m_instance, window, nullptr, &m_surface)
    //
    // 4. Feature structs for device selection:
    //      VkPhysicalDeviceVulkan12Features:  bufferDeviceAddress, scalarBlockLayout,
    //                                         descriptorIndexing (for textures later)
    //      VkPhysicalDeviceVulkan13Features:  dynamicRendering, synchronization2
    //      VkPhysicalDeviceAccelerationStructureFeaturesKHR: accelerationStructure
    //      VkPhysicalDeviceRayTracingPipelineFeaturesKHR:    rayTracingPipeline
    //
    //    vkb::PhysicalDeviceSelector: set_surface, set_minimum_version(1, 3),
    //    set_required_features_12/13, add_required_extension_features(...),
    //    add_required_extension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME),
    //    add_required_extension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME),
    //    add_required_extension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME),
    //    add_required_extension(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME)  // ImGui backend wants it explicitly
    //
    //    SER is optional: add_desired_extension(VK_NV_RAY_TRACING_INVOCATION_REORDER_EXTENSION_NAME)
    //    and check afterwards whether it was enabled → m_serSupported.
    //
    // 5. vkb::DeviceBuilder → m_vkbDevice → volkLoadDevice(m_device)
    //    Queue: get_queue(vkb::QueueType::graphics) (also presents, on NVIDIA/Windows).
    //
    // 6. VmaAllocatorCreateInfo with VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
    //    vulkanApiVersion = VK_API_VERSION_1_3, and function pointers imported from
    //    volk (vmaImportVulkanFunctionsFromVolk) since VK_NO_PROTOTYPES is defined.
    //
    // 7. Query m_rtProperties via vkGetPhysicalDeviceProperties2 and print:
    //    device name, driver version, shaderGroupHandleSize,
    //    shaderGroupBaseAlignment, shaderGroupHandleAlignment, maxRayRecursionDepth,
    //    and whether SER is available. That printout is Step 1's "done when".

    return false;
}

void VulkanContext::destroy()
{
    // Reverse order: allocator, device, surface, instance (debug messenger is
    // destroyed with the vkb::Instance).
}
