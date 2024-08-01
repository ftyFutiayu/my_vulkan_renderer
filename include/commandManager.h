#pragma once
#include "tool.h"

namespace render_2d
{
    class CommandManager final
    {
    public:
        CommandManager(uint32_t graphicsQueueFamilyIndex, VkDevice device);

        ~CommandManager();

        VkCommandBuffer allocateOneCmdBuffer();

        std::vector<VkCommandBuffer> allocateCmdBuffers(uint32_t count);

        void ResetCmdPool();

        void FreeCmdBuffer(VkCommandBuffer cmdBuffer);

    private:
        VkCommandPool pool_;

        VkCommandPool createCommandPool();

        std::vector<VkCommandBuffer> commandBuffers_;

        uint32_t graphicsQueueFamilyIndex_;

        VkDevice device_;
    };
}