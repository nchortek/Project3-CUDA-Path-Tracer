#include "pipeline.h"

#include "context.h"
#include "gpu/shared.h"

#include <array>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

bool Pipeline::init(VulkanContext& context)
{
    m_context = &context;

    if (!createRendererDescriptorSetLayout())
    {
        destroy();
        return false;
    }

    if (!createPipelineLayout())
    {
        destroy();
        return false;
    }

    if (!createPipeline())
    {
        destroy();
        return false;
    }

    if (!createShaderBindingTable())
    {
        destroy();
        return false;
    }

    return true;
}

void Pipeline::destroy()
{
    if (m_context == nullptr)
    {
        return;
    }

    // Destroy in reverse order of creation
    vmaDestroyBuffer(m_context->getAllocator(), m_sbtBuffer, m_sbtAllocation);
    m_sbtBuffer = VK_NULL_HANDLE;
    m_sbtAllocation = VK_NULL_HANDLE;

    VkDevice device = m_context->getDevice();
    vkDestroyPipeline(device, m_pipeline, nullptr);
    m_pipeline = VK_NULL_HANDLE;

    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
    m_pipelineLayout = VK_NULL_HANDLE;

    vkDestroyDescriptorSetLayout(device, m_rendererDescriptorSetLayout, nullptr);
    m_rendererDescriptorSetLayout = VK_NULL_HANDLE;

    m_raygenDeviceAddrRegion = {};
    m_missDeviceAddrRegion = {};
    m_hitDeviceAddrRegion = {};
    m_callableDeviceAddrRegion = {};
    m_context = nullptr;
}

bool Pipeline::createRendererDescriptorSetLayout()
{
    // NCHORTEK TODO: This will eventually need to include TLAS
    std::array<VkDescriptorSetLayoutBinding, 2> bindings{};

    // Configure a binding for our accumulation image (holds a running sum of samples)
    bindings.at(0).binding = gpu::kAccumImageBinding;
    bindings.at(0).descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bindings.at(0).descriptorCount = 1;
    bindings.at(0).stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    bindings.at(0).pImmutableSamplers = nullptr;

    // Configure a binding for our display image (holds the render output that gets
    // blitted into a swapchain image prior to presentation)
    bindings.at(1).binding = gpu::kDisplayImageBinding;
    bindings.at(1).descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    bindings.at(1).descriptorCount = 1;
    bindings.at(1).stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    bindings.at(1).pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    createInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_context->getDevice(), &createInfo, nullptr, &m_rendererDescriptorSetLayout)
        != VK_SUCCESS)
    {
        m_rendererDescriptorSetLayout = VK_NULL_HANDLE;
        fprintf(stderr, "Failed to create renderer descriptor set layout.\n");
        return false;
    }

    return true;
}

bool Pipeline::createPipelineLayout()
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = kPushConstantStages;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(gpu::PushConstants);

    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    
    // A descriptor set's number is its index in pSetLayouts, so enforce
    // that contract here
    std::array<VkDescriptorSetLayout, 1> setLayouts{};
    setLayouts.at(gpu::kRendererDescriptorSet) = m_rendererDescriptorSetLayout;

    createInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    createInfo.pSetLayouts = setLayouts.data();

    createInfo.pushConstantRangeCount = 1;
    createInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(m_context->getDevice(), &createInfo, nullptr, &m_pipelineLayout)
        != VK_SUCCESS)
    {
        m_pipelineLayout = VK_NULL_HANDLE;
        fprintf(stderr, "Failed to create pipeline layout.\n");
        return false;
    }

    return true;
}

bool Pipeline::createPipeline()
{
    // NCHORTEK TODO
    return true;
}

bool Pipeline::createShaderBindingTable()
{
    // NCHORTEK TODO
    return true;
}

bool Pipeline::readSpirvFile(const std::string& filename, std::vector<uint32_t>& code)
{
    // std::ios::ate instructs ifstream to start at the end of the file,
    // and std::ios::binary specifies the file to be read as its underlying
    // binary data
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        fprintf(stderr, "Failed to open shader file: %s\n", filename.c_str());
        return false;
    }

    // Because we start at the end of the file, we can quickly obtain
    // its size by obtaining the current position of the stream
    const std::streamoff fileSize = file.tellg();

    // Valid SPIR-V is a stream of 32-bit words, so sanity check that the
    // fileSize matches that expection.
    if (fileSize <= 0 || static_cast<size_t>(fileSize) % sizeof(uint32_t) != 0)
    {
        fprintf(stderr, "Invalid SPIR-V file size (%lld bytes): %s\n",
            static_cast<long long>(fileSize),
            filename.c_str());
        return false;
    }

    // Ensure our code vector is large enough to store the full SPIR-V data
    code.resize(static_cast<size_t>(fileSize) / sizeof(uint32_t));

    // Seek back to the beginning of the file
    file.seekg(0);
    file.read(reinterpret_cast<char*>(code.data()), fileSize);

    file.close();
    return true;
}

bool Pipeline::createVkShaderModule(VkDevice device, const std::vector<uint32_t>& code, VkShaderModule& shaderModule)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    // codeSize needs to be in bytes
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        shaderModule = VK_NULL_HANDLE;
        fprintf(stderr, "Failed to create shader module.\n");
        return false;
    }

    return true;
}