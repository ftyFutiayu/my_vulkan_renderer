//
// Created by 12381 on 24-7-23.
//

#include <algorithm>
#include "swapchain.h"
#include "context.h"

namespace render_2d {

    SwapChain::SwapChain() {
        querySwapChainInfo();

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        // GPU的图大于屏幕，是否裁切
        createInfo.clipped = VK_SUCCESS;
        // 交换链存在很多imageArray图片，需要制定绘制哪一个图片
        // imageArrayLayers ： 多层的图片数组，可以理解为二维数组 指定1就是不需要3D图像
        createInfo.imageArrayLayers = 1;
        // 图像使用方法 （作为颜色附件）
        createInfo.imageUsage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        // 图像显示至窗口，颜色如何blending
        createInfo.compositeAlpha = VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.surface = Context::GetInstance().surface_;

        // 图像颜色空间

    }

    SwapChain::~SwapChain() {

    }

    void SwapChain::querySwapChainInfo() {
        // 查询物理设备支持的Image Formats
        auto &physicalDevice = Context::GetInstance().physicalDevice_;
        auto &surface = Context::GetInstance().surface_;
        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());
        info.format = formats[0];

        for (const auto &format: formats) {
            // 格式为SRGB且支持非线性SRBG 颜色空间
            if (format.format == VK_FORMAT_R8G8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                info.format = format;
                break;
            }
        }

        // 查询图像个数
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
        info.imageCount = std::clamp<uint32_t>(2, capabilities.minImageCount + 1, capabilities.maxImageCount);

    }
}
