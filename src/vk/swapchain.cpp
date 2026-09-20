#include "swapchain.h"

#include "context.h"

bool Swapchain::init(VulkanContext& context, uint32_t width, uint32_t height)
{
    m_context = &context;
    return recreate(width, height);
}

bool Swapchain::recreate(uint32_t width, uint32_t height)
{
    (void)width;
    (void)height;

    // vkb::SwapchainBuilder(m_context->getVkbDevice())
    //     .set_old_swapchain(m_swapchain)
    //     .set_desired_format({ VK_FORMAT_B8G8R8A8_UNORM,        // UNORM, not SRGB:
    //                           VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })  // raygen writes gamma
    //     .set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR) // FIFO as fallback;
    //                                                            // avoid vsync when timing
    //     .set_desired_extent(width, height)
    //     .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT) // blit target (after base cmake is building)
    //     .build();
    //
    // Then destroy the old swapchain and image views, and store
    // get_images() / get_image_views().
    //
    // Note: the default usage is COLOR_ATTACHMENT (needed for the ImGui pass);
    // TRANSFER_DST is added for the blit from the display image.

    return false;
}

void Swapchain::destroy()
{
    // Destroy image views, then the swapchain.
}
