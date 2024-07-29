//
// Created by 12381 on 24-7-23.
//

#include <algorithm>
#include "../include/swapchain.h"
#include "../include/context.h"

namespace render_2d {

    SwapChain::SwapChain(int width, int height) {
        querySwapChainInfo(width, height);

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        // GPU的图大于屏幕，是否裁切
        createInfo.clipped = VK_TRUE;
        // 交换链存在很多imageArray图片，需要制定绘制哪一个图片
        // imageArrayLayers ： 多层的图片数组，可以理解为二维数组 指定1就是不需要3D图像
        createInfo.imageArrayLayers = 1;
        // 图像使用方法 （作为颜色附件）
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        // 图像显示至窗口，颜色如何blending
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.surface = Context::GetInstance().surface_;

        // 图像颜色空间
        createInfo.imageColorSpace = info.format.colorSpace;
        createInfo.imageExtent = info.imageExtent;
        createInfo.minImageCount = info.imageCount;
        createInfo.imageFormat = info.format.format;

        // Present mode
        createInfo.presentMode = info.presentMode;
        // preTransform 表示在交换链图像呈现到屏幕上之前，图像应该经历的转换
        createInfo.preTransform = info.transform;

        // 设置命令队列
        auto &queueFamilyIndices = Context::GetInstance().queueFamilyIndices_;
        if (queueFamilyIndices.graphicsQueue.value() == queueFamilyIndices.presentQueue.value()) {
            // 如果只有一个命令队列，设置图像只能被一个 queue 使用
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // 不需要设置
            createInfo.pQueueFamilyIndices = nullptr; // 不需要设置
        } else {
            std::array<uint32_t, 2> indices = {queueFamilyIndices.graphicsQueue.value(),
                                               queueFamilyIndices.presentQueue.value()};
            // 如果有多个命令队列，设置图像可以被多个 queue 使用
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.pQueueFamilyIndices = indices.data();
        }

        VkResult result = vkCreateSwapchainKHR(Context::GetInstance().device_, &createInfo,
                                               nullptr, &swapchain);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }
        std::cout << "SwapChain created successfully!" << std::endl;
    }

    SwapChain::~SwapChain() {
        std::cout << "Destroying SwapChain..." << std::endl;
        for (auto &imageView: imageViews) {
            vkDestroyImageView(Context::GetInstance().device_, imageView, nullptr);
        }
        vkDestroySwapchainKHR(Context::GetInstance().device_, swapchain, nullptr);
    }

    void SwapChain::querySwapChainInfo(int width, int height) {
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
                std::cout << " SwapChainFound suitable surface format" << std::endl;
                break;
            }
        }

        // 查询图像个数
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
        // 夹紧区间 [minImageCount, maxImageCount]
        info.imageCount = std::clamp<uint32_t>(2, capabilities.minImageCount + 1, capabilities.maxImageCount);
        std::cout << "SwapChain Image count: " << info.imageCount << std::endl;

        // 图像大小
        info.imageExtent.height = std::clamp<uint32_t>(height, capabilities.minImageExtent.height,
                                                       capabilities.maxImageExtent.height);
        info.imageExtent.width = std::clamp<uint32_t>(width, capabilities.minImageExtent.width,
                                                      capabilities.maxImageExtent.width);
        std::cout << "SwapChain Image extent width  " << info.imageExtent.width << "  Height: "
                  << info.imageExtent.height
                  << std::endl;

        // 图像传递屏幕之前的修改（旋转..）
        info.transform = capabilities.currentTransform;


        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        std::cout << "SwapChain Present mode count: " << presentModeCount << std::endl;
        std::vector<VkPresentModeKHR> presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface,
                                                  &presentModeCount,
                                                  presentModes.data());
        /*
         * Present Modes:
         * VK_PRESENT_MODE_FIFO_KHR : 先入先出 (屏幕后的图像会形成队列排队显示)
         * VK_PRESENT_MODE_FIFO_RELAXED_KHR : 也是先入先出，会存在画面撕裂，会强行放弃现在的图像
         * VK_PRESENT_MODE_IMMEDIATE_KHR : 性能最高，也会存在画面撕裂
         * VK_PRESENT_MODE_MAILBOX_KHR : 双缓冲，在Present的时候
         */

        info.presentMode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;

        for (const auto &mode: presentModes) {
            if (mode == VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR) {
                info.presentMode = mode;
                break;
            }
        }
    }

    /*
     * 获取交换链中的图像
     */
    void SwapChain::getImages() {
        uint32_t imageCount = 0;
        vkGetSwapchainImagesKHR(Context::GetInstance().device_, swapchain, &imageCount, nullptr);
        images.resize(imageCount);
        vkGetSwapchainImagesKHR(Context::GetInstance().device_, swapchain, &imageCount, images.data());
        std::cout << "SwapChain Get Image count: " << imageCount << std::endl;
    }

    /*
     * 创建交换链中的ImageView
     */
    void SwapChain::createImageViews() {
        imageViews.resize(images.size());
        for (size_t i = 0; i < imageViews.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            // mapping 实现A -> B 颜色映射 显式设置 VkComponentMapping
            VkComponentMapping mapping{
                    VK_COMPONENT_SWIZZLE_R, // r
                    VK_COMPONENT_SWIZZLE_G, // g
                    VK_COMPONENT_SWIZZLE_B, // b
                    VK_COMPONENT_SWIZZLE_A  // a
            };
            createInfo.components = mapping;
            createInfo.format = info.format.format;
            // mipmap level
            VkImageSubresourceRange subresourceRange{};
            subresourceRange.baseMipLevel = 0;
            subresourceRange.levelCount = 1; // 需要多少阶纹理？
            subresourceRange.baseArrayLayer = 0; // 3D 图像需要设置
            subresourceRange.layerCount = 1; // 3D 图像需要设置
            subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange = subresourceRange;

            createInfo.image = images[i]; // 指定当前图像
            VkResult result = vkCreateImageView(Context::GetInstance().device_, &createInfo, nullptr, &imageViews[i]);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("failed to create image view!");
            }
        }
        std::cout << "SwapChain Create Image Views successfully!" << std::endl;
    }
}
