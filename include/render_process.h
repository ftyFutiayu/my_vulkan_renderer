//
// Created by 12381 on 24-7-25.
//
#pragma once

#include "tool.h"
#include "shader.h"
#include "swapchain.h"


namespace render_2d {
    class RenderProcess final {
    public:
        RenderProcess(VkDevice& device,SwapChain& swapchain) : device_(device),swapchain_(swapchain) {
            std::cout<< "Initializing Render Process...\n";
        }

        ~RenderProcess();

        VkPipeline pipeline_;
        VkPipelineLayout layout_;
        VkRenderPass renderPass_;

        void InitLayout();
        void InitRenderPass();
        void CreatePipeline(int width, int height);
        void DestroyPipeline();

    private:
        VkDevice& device_;
        SwapChain& swapchain_;
    };
}