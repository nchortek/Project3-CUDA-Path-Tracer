#include "renderer.h"

#include "context.h"
#include "swapchain.h"

#include <cstdio>

bool Renderer::init(VulkanContext& context, Swapchain& swapchain, uint32_t width, uint32_t height)
{
    // NCHORTEK TODO
    m_context = &context;
    m_swapchain = &swapchain;
    return true;
}

void Renderer::destroy()
{
    // NCHORTEK TODO
}

void Renderer::drawFrame()
{
    // NCHORTEK TODO
}

VkCommandBuffer Renderer::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_context->getDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Renderer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkQueue graphicsQueue = m_context->getGraphicsQueue();
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(m_context->getDevice(), m_commandPool, 1, &commandBuffer);
}

bool Renderer::createCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    // We will be recording a command buffer every frame, so we want to be
    // able to reset and rerecord over it. This flag allows command buffers
    // to be rerecorded individually, without this flag they all have to be
    // reset together.
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    // Each command pool can only allocate command buffers that are submitted
    // on a single type of queue. We will use command buffers for drawing.
    poolInfo.queueFamilyIndex = m_context->getGraphicsQueueFamily();

    if (vkCreateCommandPool(m_context->getDevice(), &poolInfo, nullptr, &m_commandPool)
        != VK_SUCCESS)
    {
        fprintf(stderr, "Failed to create command pool.\n");
        return false;
    }

    return true;
}

bool Renderer::createCommandBuffers()
{
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.commandPool = m_commandPool;

    // Primary command buffers can be submitted to a queue for execution,
    // but cannot be called from other command buffers.
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandBufferCount = 1;

    VkDevice device = m_context->getDevice();
    for (FrameData& frameData : m_frames)
    {
        // Note that command buffers are automatically deallocated when the command pool that
        // created them is destroyed, so we don't need do any manual cleanup
        if (vkAllocateCommandBuffers(device, &allocateInfo, &frameData.commandBuffer)
            != VK_SUCCESS)
        {
            fprintf(stderr, "Failed to allocate command buffers.\n");
            return false;
        }
    }

    return true;
}

bool Renderer::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    // Create the fence in the signaled state, so that the first call to vkWaitForFences()
    // returns immediately since the fence is already signaled.
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkDevice device = m_context->getDevice();
    for (FrameData& frameData : m_frames)
    {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &frameData.imageAvailableSemaphore)
            != VK_SUCCESS)
        {
            fprintf(stderr, "Failed to create image-available semaphore.\n");
            return false;
        }

        if (vkCreateFence(device, &fenceInfo, nullptr, &frameData.inFlightFence) != VK_SUCCESS)
        {
            fprintf(stderr, "Failed to create in-flight fence.\n");
            return false;
        }
    }

    return true;
}

bool Renderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView)
{
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;

    // Specify how the image data should be interpreted
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;

    // // Use default color channel mapping
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    // Describe the image purpose and specify which part of the image
    // should be accessed
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;

    // Multiple array layers are only needed for stereographic 3D applications,
    // allowing different views to be displayed for each eye. A traditional
    // renderer only needs 1 layer.
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_context->getDevice(), &createInfo, nullptr, &imageView)
        != VK_SUCCESS)
    {
        imageView = VK_NULL_HANDLE;
        fprintf(stderr, "Failed to create image view.\n");
        return false;
    }

    return true;
}

bool Renderer::createStorageImage(VkFormat format, VkImageUsageFlags usage, StorageImage& storageImage)
{
    // NCHORTEK TODO
    return false;
}

void Renderer::destroyStorageImage(StorageImage& storageImage)
{
    // NCHORTEK TODO
}

bool Renderer::createStorageImages()
{
    // NCHORTEK TODO
    return false;
}

void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t swapchainImageIndex)
{
    // NCHORTEK TODO
}

void Renderer::recordImageBarrier(
    VkCommandBuffer commandBuffer,
    VkImage image,
    VkImageLayout oldImageLayout,
    VkImageLayout newImageLayout,
    VkPipelineStageFlags2 srcStage,
    VkAccessFlags2 srcAccess,
    VkPipelineStageFlags2 dstStage,
    VkAccessFlags2 dstAccess)
{
    VkImageMemoryBarrier2 barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    barrier.srcStageMask = srcStage;
    barrier.srcAccessMask = srcAccess;
    barrier.dstStageMask = dstStage;
    barrier.dstAccessMask = dstAccess;
    barrier.oldLayout = oldImageLayout;
    barrier.newLayout = newImageLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    VkDependencyInfo dependencyInfo{};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &barrier;

    vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
}