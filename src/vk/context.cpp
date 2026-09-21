#include "context.h"

#include <GLFW/glfw3.h>

#include <cstdio>

bool VulkanContext::init(GLFWwindow* window, bool enableValidation)
{
    if (volkInitialize() != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to initialize volk\n");
        return false;
    }

    if (!initInstance(enableValidation))
    {
        destroy();
        return false;
    }

    if (!initSurface(window))
    {
        destroy();
        return false;
    }

    vkb::PhysicalDevice physicalDevice;
    if (!initPhysicalDevice(physicalDevice))
    {
        destroy();
        return false;
    }

    if (!initLogicalDevice(physicalDevice))
    {
        destroy();
        return false;
    }

    if (!initVmaAllocator())
    {
        destroy();
        return false;
    }

    if (!initDeviceProperties())
    {
        destroy();
        return false;
    }

    printDeviceReport();
    return true;
}

bool VulkanContext::initInstance(bool enableValidation)
{
    auto instanceResult = vkb::InstanceBuilder()
        .set_app_name("CIS565 Path Tracer")
        .require_api_version(1, 3, 0)
        .request_validation_layers(enableValidation)
        .use_default_debug_messenger()
        .build();

    if (!instanceResult)
    {
        fprintf(stderr, "Instance creation failed: %s\n", instanceResult.error().message().c_str());
        return false;
    }

    m_vkbInstance = instanceResult.value();
    volkLoadInstance(getInstance());
    return true;
}

bool VulkanContext::initSurface(GLFWwindow* window)
{
    if (glfwCreateWindowSurface(getInstance(), window, nullptr, &m_surface) != VK_SUCCESS)
    {
        fprintf(stderr, "glfwCreateWindowSurface failed\n");
        return false;
    }

    return true;
}

bool VulkanContext::initPhysicalDevice(vkb::PhysicalDevice& physicalDevice)
{
    VkPhysicalDeviceFeatures features10{};
    features10.shaderInt64 = VK_TRUE;

    VkPhysicalDeviceVulkan12Features features12{};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.bufferDeviceAddress = VK_TRUE;
    features12.scalarBlockLayout = VK_TRUE;
    features12.descriptorIndexing = VK_TRUE;

    VkPhysicalDeviceVulkan13Features features13{};
    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.dynamicRendering = VK_TRUE;
    features13.synchronization2 = VK_TRUE;

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelFeatures{};
    accelFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    accelFeatures.accelerationStructure = VK_TRUE;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeatures{};
    rtPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    rtPipelineFeatures.rayTracingPipeline = VK_TRUE;

    auto physicalDeviceResult = vkb::PhysicalDeviceSelector(m_vkbInstance)
        .set_surface(m_surface)
        .set_minimum_version(1, 3)
        .set_required_features(features10)
        .set_required_features_12(features12)
        .set_required_features_13(features13)
        .add_required_extension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)
        .add_required_extension_features(accelFeatures)
        .add_required_extension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)
        .add_required_extension_features(rtPipelineFeatures)
        .add_required_extension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME)
        .select();

    if (!physicalDeviceResult)
    {
        fprintf(stderr, "Failed to find a suitable physical device: %s\n", physicalDeviceResult.error().message().c_str());
        return false;
    }

    physicalDevice = physicalDeviceResult.value();

    // Enable SER if its supported
    VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV serFeatures{};
    serFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_FEATURES_NV;
    serFeatures.rayTracingInvocationReorder = VK_TRUE;

    m_serSupported =
        physicalDevice.enable_extension_if_present(VK_NV_RAY_TRACING_INVOCATION_REORDER_EXTENSION_NAME)
        && physicalDevice.enable_extension_features_if_present(serFeatures);

    return true;
}

bool VulkanContext::initLogicalDevice(const vkb::PhysicalDevice& physicalDevice)
{
    auto logicalDeviceResult = vkb::DeviceBuilder(physicalDevice).build();
    if (!logicalDeviceResult)
    {
        fprintf(stderr, "Device creation failed: %s\n", logicalDeviceResult.error().message().c_str());
        return false;
    }

    m_vkbDevice = logicalDeviceResult.value();
    volkLoadDevice(getDevice());

    auto queueResult = m_vkbDevice.get_queue(vkb::QueueType::graphics);
    if (!queueResult)
    {
        fprintf(stderr, "Failed to obtain graphics queue: %s\n", queueResult.error().message().c_str());
        return false;
    }

    m_graphicsQueue = queueResult.value();

    auto queueIndexResult = m_vkbDevice.get_queue_index(vkb::QueueType::graphics);
    if (!queueIndexResult)
    {
        fprintf(stderr, "Failed to obtain graphics queue index: %s\n", queueIndexResult.error().message().c_str());
        return false;
    }

    m_graphicsQueueFamily = queueIndexResult.value();

    return true;
}

bool VulkanContext::initVmaAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    allocatorInfo.instance = m_vkbInstance.instance;
    allocatorInfo.physicalDevice = getPhysicalDevice();
    allocatorInfo.device = getDevice();
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

    VmaVulkanFunctions vmaFunctions{};
    if (vmaImportVulkanFunctionsFromVolk(&allocatorInfo, &vmaFunctions) != VK_SUCCESS)
    {
        fprintf(stderr, "vmaImportVulkanFunctionsFromVolk failed\n");
        return false;
    }

    allocatorInfo.pVulkanFunctions = &vmaFunctions;

    if (vmaCreateAllocator(&allocatorInfo, &m_allocator) != VK_SUCCESS)
    {
        fprintf(stderr, "vmaCreateAllocator failed\n");
        return false;
    }

    return true;
}

bool VulkanContext::initDeviceProperties()
{
    m_driverProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
    m_rtProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

    VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV serProperties{};
    serProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_PROPERTIES_NV;

    VkPhysicalDeviceProperties2 properties2{};
    properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;

    properties2.pNext = &m_rtProperties;
    m_rtProperties.pNext = &m_driverProperties;
    m_driverProperties.pNext = m_serSupported ? &serProperties : nullptr;

    vkGetPhysicalDeviceProperties2(getPhysicalDevice(), &properties2);
    m_deviceProperties = properties2.properties;

    if (m_serSupported)
    {
        m_serReorderMode = serProperties.rayTracingInvocationReorderReorderingHint;
    }

    m_rtProperties.pNext = nullptr;
    m_driverProperties.pNext = nullptr;

    return true;
}

void VulkanContext::printDeviceReport() const
{
    printf("GPU: %s\n", m_deviceProperties.deviceName);
    printf("Driver: %s %s\n", m_driverProperties.driverName, m_driverProperties.driverInfo);
    printf("Highest Vulkan API supported (app requires 1.3): %u.%u.%u\n",
        VK_API_VERSION_MAJOR(m_deviceProperties.apiVersion),
        VK_API_VERSION_MINOR(m_deviceProperties.apiVersion),
        VK_API_VERSION_PATCH(m_deviceProperties.apiVersion));
    printf("shaderGroupHandleSize: %u\n", getRtProperties().shaderGroupHandleSize);
    printf("shaderGroupBaseAlignment: %u\n", getRtProperties().shaderGroupBaseAlignment);
    printf("shaderGroupHandleAlignment: %u\n", getRtProperties().shaderGroupHandleAlignment);
    printf("maxRayRecursionDepth: %u\n", getRtProperties().maxRayRecursionDepth);
    printf("SER (NV reorder): %s\n",
        m_serSupported
        ? (m_serReorderMode == VK_RAY_TRACING_INVOCATION_REORDER_MODE_REORDER_NV
            ? "extension present, reordering enabled" : "extension present, reordering disabled")
        : "extension not present");
}

void VulkanContext::destroy()
{
    // Destroy in reverse order of creation:
    // allocator --> device --> surface --> instance
    if (VkDevice device = getDevice(); device != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(device);
    }

    if (m_allocator != VK_NULL_HANDLE)
    {
        vmaDestroyAllocator(m_allocator);
        m_allocator = nullptr;
    }

    vkb::destroy_device(m_vkbDevice);

    if (m_surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(getInstance(), m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }

    // (debug messenger is destroyed with the vkb::Instance).
    vkb::destroy_instance(m_vkbInstance);
    m_vkbDevice = {};
    m_vkbInstance = {};
}
