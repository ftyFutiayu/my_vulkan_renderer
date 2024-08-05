#include <limits>
#include "../include/renderer.h"

namespace render_2d
{
    const std::array<Vertex, 3> vertices = {
        Vertex{0.0, -0.5},
        Vertex{0.5, 0.5},
        Vertex{-0.5, 0.5},
    };

    // 同步 CPU和 GPU
    void Renderer::createFences()
    {
        fences_.resize(maxFlightCount_);
        for (auto &fence : fences_)
        {
            VkFenceCreateInfo fenceInfo{};
            // VK_FENCE_CREATE_SIGNALED_BIT : 初始状态为已签发
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            vkCreateFence(device_, &fenceInfo, nullptr, &fence);
        }
        // 检查 fence状态
        for (size_t i = 0; i < maxFlightCount_; ++i)
        {
            auto res = vkGetFenceStatus(device_, fences_[i]);
            if (res != VK_SUCCESS)
            {
                std::cerr << "Renderer Fence " << i << " not signaled!" << std::endl;
            }
        }
        std::cout << "Renderer createFences success" << std::endl;
    }

    void Renderer::createSemaphores()
    {
        imageAvaliableSems_.resize(maxFlightCount_);
        renderFinishSems_.resize(maxFlightCount_);
        VkResult res;
        for (size_t i = 0; i < maxFlightCount_; ++i)
        {
            VkSemaphoreCreateInfo availableSemsInfo{};
            availableSemsInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            res = vkCreateSemaphore(device_, &availableSemsInfo, nullptr, &imageAvaliableSems_[i]);
            if (res != VK_SUCCESS)
            {
                std::cerr << "Renderer imageAvaliableSems_ " << i << " not created!" << std::endl;
            }
            VkSemaphoreCreateInfo renderFinishSemsInfo{};
            renderFinishSemsInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            res = vkCreateSemaphore(device_, &renderFinishSemsInfo, nullptr, &renderFinishSems_[i]);
            if (res != VK_SUCCESS)
            {
                std::cerr << "Renderer imageAvaliableSems_ " << i << " not created!" << std::endl;
            }
        }
        std::cerr << "Render createSemaphores success" << std::endl;
    }

    void Renderer::createCmdBuffers()
    {
        cmdBufs_.resize(maxFlightCount_);
        for (auto &buf : cmdBufs_)
        {
            buf = commandManager_->allocateOneCmdBuffer();
        }
        std::cerr << "Render createCmdBuffers success size ->" << maxFlightCount_ << std::endl;
    }

    void Renderer::DrawTriangle()
    {
        if (vkWaitForFences(device_, 1, &fences_[curFrame_], VK_TRUE, std::numeric_limits<uint64_t>::max()) != VK_SUCCESS)
        {
            throw std::runtime_error("wait for fence failed");
        }
        vkResetFences(device_, 1, &fences_[curFrame_]);

        // 2.查询交换链中下一个空 image
        uint32_t imageIndex;

        auto res = vkAcquireNextImageKHR(device_, swapChain_->swapchain,
                                         std::numeric_limits<uint64_t>::max(), VK_NULL_HANDLE, VK_NULL_HANDLE,
                                         &imageIndex);

        if (res != VK_SUCCESS)
        {
            std::cerr << "Render Failed to acquire swap chain image" << std::endl;
            return;
        }

        // 3. 重置 CommandBuffer
        vkResetCommandBuffer(cmdBufs_[curFrame_], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

        // 4. 开始记录 CommandBuffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // USAGE_ONE_TIME_SUBMIT_BIT : commandBuffer只执行一次，后续不再使用
        // USAGE_RENDER_PASS_CONTINUE_BIT : commandBuffer在renderPass整个声明周期都要存在
        // USAGE_SIMULTANEOUS_USE_BIT : 可以重复使用 （用于多个线程中同时使用同一个命令缓冲区，commandBuffer不能改变)
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmdBufs_[curFrame_], &beginInfo);

        // 4. 开始执行 RenderPass ，通过执行 vkCmdBeginRenderPass
        vkCmdBindPipeline(cmdBufs_[curFrame_], VK_PIPELINE_BIND_POINT_GRAPHICS, renderProcess_->pipeline_); // 绑定pipeline
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
        vkCmdBeginRenderPass(cmdBufs_[curFrame_], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        /* 5. 使用GPU传入的顶点参数进行渲染 (多个buffer需要进行偏移) */
        VkDeviceSize vertexBufferOffset = 0;
        vkCmdBindVertexBuffers(cmdBufs_[curFrame_], 0, 1, &vertexBuffer_->buffer_, &vertexBufferOffset);
        // 执行 Draw 命令
        vkCmdDraw(cmdBufs_[curFrame_], 3, 1, 0, 0);

        // 6. 结束记录 renderPass && CommandBuffer
        vkCmdEndRenderPass(cmdBufs_[curFrame_]);
        res = vkEndCommandBuffer(cmdBufs_[curFrame_]);
        if (res != VK_SUCCESS)
        {
            std::cerr << "Render Failed to end command buffer" << std::endl;
        }

        // 指定提交至GPU 图形队列以及显示队列
        VkSubmitInfo submitGraphicsInfo{};
        submitGraphicsInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitGraphicsInfo.commandBufferCount = 1;
        submitGraphicsInfo.pCommandBuffers = &cmdBufs_[curFrame_];
        submitGraphicsInfo.signalSemaphoreCount = 1;
        submitGraphicsInfo.pSignalSemaphores = &renderFinishSems_[curFrame_];
        submitGraphicsInfo.waitSemaphoreCount = 1;
        submitGraphicsInfo.pWaitSemaphores = &imageAvaliableSems_[curFrame_];

        res = vkQueueSubmit(graphicsQueue_, 1, &submitGraphicsInfo, fences_[curFrame_]);
        if (res != VK_SUCCESS)
        {
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
        if (res != VK_SUCCESS)
        {
            std::cerr << "Render Failed to present to screen" << std::endl;
            return;
        }

        curFrame_ = (curFrame_ + 1) % maxFlightCount_;
    }

    // 如何创建顶点buffer
    void Renderer::createVertexBuffer()
    {
        // 创建并填充 Vertex Buffer
        // VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 用于创建 vertex buffer (其他store..uniform...indirect)
        // 一定要设置  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT , 要求内存对宿主机(CPU)可见
        vertexBuffer_ = std::make_unique<Buffer>(sizeof(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, device_, gpu_);
        std::cout << "Renderer create VertexBuffer success" << std::endl;
    }

    // 如何将顶点传入到GPU
    void Renderer::bufferVertexData()
    {
        void *ptr;
        // 映射内存
        vkMapMemory(device_, vertexBuffer_->memory_, 0, sizeof(vertices), 0, &ptr);
        memcpy(ptr, vertices.data(), sizeof(vertices));
        vkUnmapMemory(device_, vertexBuffer_->memory_);
        std::cout << "Renderer bufferVertexData success" << std::endl;
    }

    Renderer::~Renderer()
    {
        vertexBuffer_.reset();
        std::cout << "Destroy Vulkan Renderer" << std::endl;
        for (auto &imageSem : imageAvaliableSems_)
        {
            vkDestroySemaphore(device_, imageSem, nullptr);
        }
        for (auto &finishSem : renderFinishSems_)
        {
            vkDestroySemaphore(device_, finishSem, nullptr);
        }
        for (auto &fence : fences_)
        {
            vkDestroyFence(device_, fence, nullptr);
        }
    }
}