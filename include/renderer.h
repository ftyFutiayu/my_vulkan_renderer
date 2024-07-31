#pragma once

#include <utility>

#include "vulkan/vulkan.h"
#include "swapchain.h"
#include "render_process.h"

namespace render_2d {
    class Renderer final {
    public:
        Renderer(VkDevice logicDevice, std::shared_ptr<SwapChain> swapChain,
                 std::shared_ptr<RenderProcess> renderProcess, VkQueue graphicsQueue, VkQueue presentQueue,
                 uint32_t graphicsQueueIndex, uint32_t presentQueueIndex)
                : device_(logicDevice), swapChain_(std::move(swapChain)), renderProcess_(std::move(renderProcess)),
                  graphicsQueue_(graphicsQueue), presentQueue_(presentQueue),
                  graphicsQueueIndex_(graphicsQueueIndex), presentQueueIndex_(presentQueueIndex) {

            initCommandPool();
            allocCommandBuffer();
            createSems();
            createFence();
            std::cout << "Renderer initialized" << std::endl;
        }

        ~Renderer();

        void Render();

    private:
        VkCommandPool cmdPool_;

        VkCommandBuffer cmdBuffer_;

        VkFence availableFence_;

        VkSemaphore imageAvaliable_;

        VkSemaphore imageDrawFinish_;

        void initCommandPool();

        void allocCommandBuffer();

        void createFence();

        void createSems();

        VkDevice device_;

        std::shared_ptr<SwapChain> swapChain_;

        std::shared_ptr<RenderProcess> renderProcess_;

        VkQueue graphicsQueue_;

        VkQueue presentQueue_;

        uint32_t graphicsQueueIndex_;

        uint32_t presentQueueIndex_;
    };
}
