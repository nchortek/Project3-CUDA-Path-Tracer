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
    // NCHORTEK TODO
    return {};
}

void Renderer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    // NCHORTEK TODO
}

bool Renderer::createCommandPool()
{
    // NCHORTEK TODO
    return false;
}

bool Renderer::createCommandBuffers()
{
    // NCHORTEK TODO
    return false;
}

bool Renderer::createSyncObjects()
{
    // NCHORTEK TODO
    return false;
}

VkImageView Renderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    // NCHORTEK TODO
    return {};
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