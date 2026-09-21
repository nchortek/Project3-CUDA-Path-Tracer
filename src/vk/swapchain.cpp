#include "swapchain.h"

#include "context.h"

#include <cstdio>

bool Swapchain::init(VulkanContext& context, uint32_t width, uint32_t height)
{
    m_context = &context;
    return recreate(width, height);
}

void Swapchain::cleanupPerImageResources()
{
    if (m_context == nullptr)
    {
        return;
    }

    // Destroy image views
    if (VkDevice device = m_context->getDevice(); device != VK_NULL_HANDLE)
    {
        for (VkImageView imgView : m_imageViews)
        {
            vkDestroyImageView(device, imgView, nullptr);
        }
    }

    // NCHORTEK TODO: Later we'll need to clean up per-image semaphores

    m_imageViews.clear();
    m_images.clear();
}

bool Swapchain::recreate(uint32_t width, uint32_t height)
{
    if (m_context == nullptr)
    {
        fprintf(stderr, "Attempted to create swapchain without a valid VulkanContext\n");
        return false;
    }

    if (VkDevice device = m_context->getDevice(); device != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(device);
    }

    vkb::Swapchain oldSwapchain = m_vkbSwapchain;
    cleanupPerImageResources();

    auto swapchainResult = vkb::SwapchainBuilder(m_context->getVkbDevice())
        .set_old_swapchain(oldSwapchain)
        .set_desired_format({
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        })
        .set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
        .set_desired_extent(width, height)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build();

    vkb::destroy_swapchain(oldSwapchain);

    if (!swapchainResult)
    {
        m_vkbSwapchain = {};
        fprintf(stderr, "Swapchain build failed: %s\n", swapchainResult.error().message().c_str());
        return false;
    }

    m_vkbSwapchain = swapchainResult.value();

    auto imagesResult = m_vkbSwapchain.get_images();
    if (!imagesResult)
    {
        fprintf(stderr, "Swapchain get_images failed: %s\n", imagesResult.error().message().c_str());
        destroy();
        return false;
    }

    m_images = std::move(imagesResult.value());

    auto viewsResult = m_vkbSwapchain.get_image_views();
    if (!viewsResult)
    {
        fprintf(stderr, "Swapchain get_image_views failed: %s\n", viewsResult.error().message().c_str());
        destroy();
        return false;
    }

    m_imageViews = std::move(viewsResult.value());

    return true;
}

void Swapchain::destroy()
{
    // Destroy per-image resources before the swapchain
    cleanupPerImageResources();
    vkb::destroy_swapchain(m_vkbSwapchain);
    m_vkbSwapchain = {};
}
