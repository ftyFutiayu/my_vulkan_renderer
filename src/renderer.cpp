#include <limits>
#include "../include/renderer.h"

namespace render_2d {
    void Renderer::initCommandPool() {
        VkCommandPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        // VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : 从该 CommandPool 创建的 CommandBuffer瞬间触发，可以一帧内反复使用
        // VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : 允许 CommandBuffer单独进行reset，不然只能整个池子一起reset
        createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        createInfo.queueFamilyIndex = graphicsQueueIndex_;
        vkCreateCommandPool(device_, &createInfo, nullptr, &cmdPool_);
        std::cout << "Render Command Pool Created" << std::endl;
    }

    // 从 commandPool中分配一个或多个 commandBuffer
    void Renderer::allocCommandBuffer() {
        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.commandPool = cmdPool_;
        // level == VK_COMMAND_BUFFER_LEVEL_PRIMARY ，可以直接传递GPU执行命令
        // level == VK_COMMAND_BUFFER_LEVEL_SECONDARY ，可以作为子命令提交到 CommandBuffer中执行
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandBufferCount = 1;
        vkAllocateCommandBuffers(device_, &allocateInfo, &cmdBuffer_);
        std::cout << "Render allocCommandBuffer Success" << std::endl;
    }

    // 同步 CPU和 GPU
    void Renderer::createFence() {
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        // VK_FENCE_CREATE_SIGNALED_BIT : 初始状态为已签发
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(device_, &fenceInfo, nullptr, &availableFence_);
        std::cout << "Render createFence success" << std::endl;
    }

    void Renderer::createSems() {
        VkSemaphoreCreateInfo availableSemsInfo{};
        availableSemsInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        vkCreateSemaphore(device_, &availableSemsInfo, nullptr, &imageAvaliable_);
        std::cout << "Render createImage AvailableSemaphore success" << std::endl;

        VkSemaphoreCreateInfo drawFinishSemsInfo{};
        drawFinishSemsInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        vkCreateSemaphore(device_, &drawFinishSemsInfo, nullptr, &imageDrawFinish_);
        std::cout << "Render createImage imageDrawFinish_ success" << std::endl;
    }

    void Renderer::Render() {
        // 1. vkImage被绑定到 FrameBuffer，这里需要查询下一个空的 image用于绘制
        uint32_t imageIndex;

        auto res = vkAcquireNextImageKHR(device_, swapChain_->swapchain,
                                         std::numeric_limits<uint64_t>::max(), VK_NULL_HANDLE, VK_NULL_HANDLE,
                                         &imageIndex);

        if (res != VK_SUCCESS) {
            std::cerr << "Render Failed to acquire swap chain image" << std::endl;
            return;
        }

        // 2. 重置 CommandBuffer
        vkResetCommandBuffer(cmdBuffer_, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

        // 3. 开始记录 CommandBuffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // USAGE_ONE_TIME_SUBMIT_BIT : commandBuffer只执行一次，后续不再使用
        // USAGE_RENDER_PASS_CONTINUE_BIT : commandBuffer在renderPass整个声明周期都要存在
        // USAGE_SIMULTANEOUS_USE_BIT : 可以重复使用 （用于多个线程中同时使用同一个命令缓冲区，commandBuffer不能改变)
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmdBuffer_, &beginInfo);

        // 4. 开始执行 RenderPass ，通过执行 vkCmdBeginRenderPass
        vkCmdBindPipeline(cmdBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, renderProcess_->pipeline_); // 绑定pipeline
        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderProcess_->renderPass_;
        VkRect2D renderAreaExtent{};
        renderAreaExtent.extent = swapChain_->info.imageExtent;
        renderAreaExtent.offset.x = 0;
        renderAreaExtent.offset.y = 0;
        renderPassBeginInfo.renderArea = renderAreaExtent; // renderPass在屏幕作用位置
        renderPassBeginInfo.framebuffer = swapChain_->framebuffers[imageIndex];
        VkClearValue clearValue{};
        clearValue.color = {1.0f, 1.0f, 1.0f, 1.0f};
        renderPassBeginInfo.pClearValues = &clearValue;
        /*
            VK_SUBPASS_CONTENTS_INLINE :
            主命令缓冲区将直接包含所有必要的绘制和渲染命令,无需使用次级cmd，子通道内容会直接嵌入并记录主cmd
            VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS：
            通道的内容将被记录到一个或多个次级命令缓冲区中,可以创建多个次级命令缓冲区，每个都包含一组特定的渲染命令，然后在需要时从主命令缓冲区中调用它们
        */
        vkCmdBeginRenderPass(cmdBuffer_, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        // 5. 执行 Draw 命令
        vkCmdDraw(cmdBuffer_, 3, 1, 0, 0);

        // 6. 结束记录 renderPass && CommandBuffer
        vkCmdEndRenderPass(cmdBuffer_);
        res = vkEndCommandBuffer(cmdBuffer_);
        if (res != VK_SUCCESS) {
            std::cerr << "Render Failed to end command buffer" << std::endl;
        }

        VkSubmitInfo submitGraphicsInfo{};
        submitGraphicsInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitGraphicsInfo.commandBufferCount = 1;
        submitGraphicsInfo.pCommandBuffers = &cmdBuffer_;
        submitGraphicsInfo.signalSemaphoreCount = 1;
        submitGraphicsInfo.pSignalSemaphores = &imageDrawFinish_;
        submitGraphicsInfo.waitSemaphoreCount = 1;
        submitGraphicsInfo.pWaitSemaphores = &imageAvaliable_;

        res = vkQueueSubmit(graphicsQueue_, 1, &submitGraphicsInfo, availableFence_);
        if (res != VK_SUCCESS) {
            std::cerr << "Render Failed to submit graphics queue res: " << res << std::endl;
            return;
        }

        // 7. 交换数据并提交 GPU
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapChain_->swapchain;
        res = vkQueuePresentKHR(presentQueue_, &presentInfo);
        if (res != VK_SUCCESS) {
            std::cerr << "Render Failed to present to screen" << std::endl;
            return;
        }

        // 8. 等待Fence
        res = vkWaitForFences(device_, 1, &availableFence_, VK_TRUE, std::numeric_limits<uint64_t>::max());
        if (res != VK_SUCCESS) {
            std::cerr << "Render Waiting For AvailableFence" << std::endl;
        }
        vkResetFences(device_, 1, &availableFence_);
    }


    Renderer::~Renderer() {
        std::cout << "Destroy Vulkan Renderer" << std::endl;
        vkDestroySemaphore(device_, imageAvaliable_, nullptr);
        vkDestroySemaphore(device_, imageDrawFinish_, nullptr);
        vkDestroyFence(device_, availableFence_, nullptr);
        vkFreeCommandBuffers(device_, cmdPool_, 1, &cmdBuffer_);
        vkDestroyCommandPool(device_, cmdPool_, nullptr);
    }
}