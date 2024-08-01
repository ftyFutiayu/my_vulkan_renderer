#pragma once

#include <utility>
#include "tool.h"
#include "swapchain.h"
#include "render_process.h"
#include "commandManager.h"

namespace render_2d
{
    class Renderer final
    {
    public:
        Renderer(VkDevice logicDevice, std::shared_ptr<SwapChain> swapChain,
                 std::shared_ptr<RenderProcess> renderProcess, VkQueue graphicsQueue,
                 VkQueue presentQueue, std::shared_ptr<CommandManager> commandManager)
            : device_(logicDevice), swapChain_(std::move(swapChain)), renderProcess_(std::move(renderProcess)),
              graphicsQueue_(graphicsQueue), presentQueue_(presentQueue),
              commandManager_(commandManager)
        {
            maxFlightCount_ = swapChain_->images.size() - 1;
            createFences();
            createSemaphores();
            createCmdBuffers();
            std::cout << "Renderer initialized flightCount ->" << maxFlightCount_ << std::endl;
        }

        ~Renderer();

        void DrawTriangle();

    private:
        void createFences();

        void createSemaphores();

        void createCmdBuffers();

    private:
        std::vector<VkFence> fences_;

        std::vector<VkSemaphore> imageAvaliableSems_;

        std::vector<VkSemaphore> renderFinishSems_;

        std::vector<VkCommandBuffer> cmdBufs_;

        int maxFlightCount_;

        int curFrame_ = 0;

        VkDevice device_;

        std::shared_ptr<SwapChain> swapChain_;

        std::shared_ptr<RenderProcess> renderProcess_;

        std::shared_ptr<CommandManager> commandManager_;

        VkQueue graphicsQueue_;

        VkQueue presentQueue_;
    };
}
