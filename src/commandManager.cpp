#include "../include/commandManager.h"

namespace render_2d
{

    CommandManager::CommandManager(uint32_t graphicsQueueFamilyIndex, VkDevice device)
    {
        std::cerr << "CommandManager created" << std::endl;
        graphicsQueueFamilyIndex_ = graphicsQueueFamilyIndex;
        device_ = device;
        pool_ = createCommandPool();
    }

    CommandManager::~CommandManager()
    {
        std::cerr << "CommandManager destroyed" << std::endl;
        vkDestroyCommandPool(device_, pool_, nullptr);
    }

    VkCommandBuffer CommandManager::allocateOneCmdBuffer()
    {
        // std::cerr << "CommandManager allocateOneCmdBuffer" << std::endl;
        return allocateCmdBuffers(1)[0];
    }

    // 根据 SwapChain的图片数量来创建Command Buffers,每一帧GPU图像单独创建 cmdBuffer
    std::vector<VkCommandBuffer> CommandManager::allocateCmdBuffers(uint32_t count)
    {
        std::vector<VkCommandBuffer> cmdBuffers(count);

        VkCommandBufferAllocateInfo cmdInfo{};
        cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdInfo.commandBufferCount = count;
        cmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdInfo.commandPool = pool_;

        vkAllocateCommandBuffers(device_, &cmdInfo, cmdBuffers.data());
        // std::cerr << "CommandManager cmdBuffers allocated , buffer size ->" << count << std::endl;
        return cmdBuffers;
    }

    VkCommandPool CommandManager::createCommandPool()
    {
        VkCommandPoolCreateInfo cmdPoolCreateInfo{};
        cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        // set 指定图形队列
        cmdPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_;
        vkCreateCommandPool(device_, &cmdPoolCreateInfo, nullptr, &pool_);
        std::cerr << "CommandManager cmdPool created cur_queueFamilyIndex : "
                  << graphicsQueueFamilyIndex_ << std::endl;
        return pool_;
    }

    void CommandManager::ResetCmdPool()
    {
        vkResetCommandPool(device_, pool_, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
    }

    void CommandManager::FreeCmdBuffer(VkCommandBuffer cmdBuffer)
    {
        vkFreeCommandBuffers(device_, pool_, 1, &cmdBuffer);
    }
}
