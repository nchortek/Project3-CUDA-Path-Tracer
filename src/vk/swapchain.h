#pragma once

// Owns the swapchain and its image views. The window is fixed at the scene's
// resolution and non-resizable, so recreate() is only needed for minimize and
// VK_ERROR_OUT_OF_DATE_KHR.

#include <volk.h>

#include <vector>

class VulkanContext;

class Swapchain
{
public:
    bool init(VulkanContext& context, uint32_t width, uint32_t height);
    bool recreate(uint32_t width, uint32_t height);
    void destroy();

    VkSwapchainKHR getHandle() const
    {
	    return m_swapchain;
    }

    // This should be UNORM--gamma is applied in raygen
    VkFormat getFormat() const
    {
	    return m_format;
    }

    VkExtent2D getExtent() const
    {
	    return m_extent;
    }

    uint32_t getImageCount() const
    {
        return static_cast<uint32_t>(m_images.size());
    }

    VkImage getImage(uint32_t idx) const
    {
        return m_images[idx];
    }

    VkImageView getImageView(uint32_t idx) const
    {
        return m_imageViews[idx];
    }

private:
    VulkanContext* m_context = nullptr;

    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkFormat m_format = VK_FORMAT_UNDEFINED;
    VkExtent2D m_extent{};

    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
};
