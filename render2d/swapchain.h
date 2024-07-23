//
// Created by 12381 on 24-7-23.
//

#pragma once
#include "tool.h"

namespace render_2d {
    class SwapChain final {
    public:
        VkSwapchainKHR swapchain;

        SwapChain();

        ~SwapChain();

        struct SwapChainInfo {
            // 图像尺寸
            VkExtent2D imageExtent;
            // 图像数量
            uint32_t imageCount;
            // 图像格式
            VkSurfaceFormatKHR format;
        };

        SwapChainInfo info;
        void querySwapChainInfo();
    };
}