#pragma once

#include <volk.h>
#include <vk_mem_alloc.h>

#include <array>
#include <cstdint>

class VulkanContext;
class Swapchain;

class Renderer
{
public:
    static constexpr uint32_t kFramesInFlight = 2;
    static constexpr uint64_t kMaxTimeout = UINT64_MAX;

    Renderer() = default;
    ~Renderer()
    {
        destroy();
    }

    // Renderer should be created once and passed by reference as needed,
    // so we should disable copies/moves
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    bool init(VulkanContext& context, Swapchain& swapchain, uint32_t width, uint32_t height);
    void destroy();
    bool drawFrame();

private:
    VulkanContext* m_context = nullptr;
    Swapchain* m_swapchain = nullptr;

    VkCommandPool m_commandPool = VK_NULL_HANDLE;

    struct FrameData
    {
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
        VkFence inFlightFence = VK_NULL_HANDLE;
    };

    std::array<FrameData, kFramesInFlight> m_frames{};
    uint32_t m_frameInFlight = 0;

    struct StorageImage
    {
        VkImage image = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;
        VkFormat format = VK_FORMAT_UNDEFINED;
    };

    StorageImage m_accumImage{};
    StorageImage m_displayImage{};
    VkExtent2D m_renderExtent{};

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    bool createCommandPool();
    bool createCommandBuffers();
    bool createSyncObjects();
    bool createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView);

    bool createStorageImages();
    bool createStorageImage(VkFormat format, VkImageUsageFlags usage, StorageImage& storageImage);
    void destroyStorageImage(StorageImage& storageImage);

    bool recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t swapchainImageIndex);

    static void recordImageBarrier(
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkPipelineStageFlags2 srcStage,
        VkAccessFlags2 srcAccess,
        VkPipelineStageFlags2 dstStage,
        VkAccessFlags2 dstAccess);
};