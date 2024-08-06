#pragma once

#include <utility>
#include "tool.h"
#include "swapchain.h"
#include "render_process.h"
#include "commandManager.h"
#include "vertex.h"
#include "buffer.h"

namespace render_2d
{
    class Renderer final
    {
    public:
        Renderer(VkDevice logicDevice, VkPhysicalDevice gpu, std::shared_ptr<SwapChain> swapChain,
                 std::shared_ptr<RenderProcess> renderProcess, VkQueue graphicsQueue,
                 VkQueue presentQueue, std::shared_ptr<CommandManager> commandManager)
            : device_(logicDevice), swapChain_(std::move(swapChain)), renderProcess_(std::move(renderProcess)),
              graphicsQueue_(graphicsQueue), presentQueue_(presentQueue),
              commandManager_(commandManager), gpu_(gpu)
        {
            maxFlightCount_ = swapChain_->images.size() - 1;
            std::cout << "Renderer initialized flightCount ->" << maxFlightCount_ << std::endl;
            createFences();
            createSemaphores();
            createCmdBuffers();
            createVertexBuffer();
            bufferVertexData();
        }

        ~Renderer();

        void DrawTriangle();

    private:
        std::vector<VkFence> fences_;

        std::vector<VkSemaphore> imageAvaliableSems_;

        std::vector<VkSemaphore> renderFinishSems_;

        std::vector<VkCommandBuffer> cmdBufs_;

        int maxFlightCount_;

        int curFrame_ = 0;

        std::unique_ptr<Buffer> hostVertexBuffer_; // 使用CPU&GPU共享IO内存 buffer

        std::unique_ptr<Buffer> deviceVertexBuffer_; // GPU独占的buffer

        void createFences();

        void createSemaphores();

        void createCmdBuffers();

        void createVertexBuffer();

        void bufferVertexData();

    private:
        VkDevice device_;

        VkPhysicalDevice gpu_;

        std::shared_ptr<SwapChain> swapChain_;

        std::shared_ptr<RenderProcess> renderProcess_;

        std::shared_ptr<CommandManager> commandManager_;

        VkQueue graphicsQueue_;

        VkQueue presentQueue_;
    };
}
