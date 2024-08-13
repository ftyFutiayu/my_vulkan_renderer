//
// Created by 12381 on 24-7-25.
//
#pragma once

#include "tool.h"
#include "shader.h"
#include "swapchain.h"
#include "vertex.h"

namespace render_2d {
    class RenderProcess final {
    public:
        RenderProcess(VkDevice &device, SwapChain &swapchain, Shader &shader);

        ~RenderProcess();

        VkPipeline pipeline_;
        VkPipelineLayout layout_;
        VkRenderPass renderPass_;

    private:
        void initLayout(Shader &shader);

        void initRenderPass();

        void createPipeline(Shader &shader);

    private:
        VkDevice &device_;
        SwapChain &swapchain_;
    };
}