#include <limits>
#include "../include/renderer.h"

namespace render_2d {
    const std::array<glm::vec2, 4> vertices = {
            glm::vec2(-0.5, 0.5),
            glm::vec2(0.5, 0.5),
            glm::vec2(0.5, -0.5),
            glm::vec2(-0.5, -0.5)
    };

    const std::uint32_t indices[] = {0, 3, 1, 1, 3, 2};

    const Color initColor{0.0f, 1.0f, 0.0f, 1.0f};

    Renderer::Renderer(int maxFlightCount) : maxFlightCount_(maxFlightCount), curFrame_(0) {
        createFences();
        createSemaphores();
        createCmdBuffers();
        createVertexIndexBuffer();
        // vertex & indices buffer -> GPU device memory
        bufferVertexData();
        createUniformBuffers();

        createDescriptorPool();
        allocateDescriptorSets();
        updateDescriptorSets();
        initMats();

        SetDrawColor(initColor);
    }

    Renderer::~Renderer() {
        std::cout << "Destroy Vulkan Renderer" << std::endl;
        auto &device = Context::GetInstance().device_;

        vkDestroyDescriptorPool(device, colorDescriptorPool_, nullptr);
        vkDestroyDescriptorPool(device, mvpDescriptorPool_, nullptr);
        
        hostVertexBuffer_.reset();
        deviceVertexBuffer_.reset();
        hostIndicesBuffer_.reset();
        deviceIndicesBuffer_.reset();

        for (auto &buffer: hostMVPUniformBufs_) {
            buffer.reset();
        }
        for (auto &buffer: localMVPUniformBufs_) {
            buffer.reset();
        }
        for (auto &buffer: hostColorUniformBufs) {
            buffer.reset();
        }
        for (auto &buffer: localColorUniformBufs_) {
            buffer.reset();
        }
        for (auto &imageSem: imageAvaliableSems_) {
            vkDestroySemaphore(device, imageSem, nullptr);
        }
        for (auto &finishSem: renderFinishSems_) {
            vkDestroySemaphore(device, finishSem, nullptr);
        }
        for (auto &fence: fences_) {
            vkDestroyFence(device, fence, nullptr);
        }
    }

    // 同步 CPU和 GPU
    void Renderer::createFences() {
        auto &device = Context::GetInstance().device_;
        fences_.resize(maxFlightCount_);

        for (auto &fence: fences_) {
            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            // VK_FENCE_CREATE_SIGNALED_BIT : 初始状态为已签发
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            vkCreateFence(device, &fenceInfo, nullptr, &fence);
        }

        std::cout << "Renderer createFences success" << std::endl;
    }

    void Renderer::createSemaphores() {
        auto &device = Context::GetInstance().device_;

        imageAvaliableSems_.resize(maxFlightCount_);
        renderFinishSems_.resize(maxFlightCount_);
        VkResult res;
        for (size_t i = 0; i < maxFlightCount_; ++i) {
            VkSemaphoreCreateInfo availableSemsInfo{};
            availableSemsInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            vkCreateSemaphore(device, &availableSemsInfo, nullptr, &imageAvaliableSems_[i]);

            VkSemaphoreCreateInfo renderFinishSemsInfo{};
            renderFinishSemsInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            vkCreateSemaphore(device, &renderFinishSemsInfo, nullptr, &renderFinishSems_[i]);
     
        }
        std::cerr << "Render createSemaphores success" << std::endl;
    }

    void Renderer::createCmdBuffers() {
        auto &cmd = Context::GetInstance().commandManager_;
        cmdBufs_.resize(maxFlightCount_);
        for (auto &buf: cmdBufs_) {
            buf = cmd->allocateOneCmdBuffer();
        }
        std::cerr << "Render createCmdBuffers success size ->" << maxFlightCount_ << std::endl;
    }

    void Renderer::DrawRect(const Rect &rect) {
        auto &ctx = Context::GetInstance();
        auto &device = ctx.device_;
        auto &renderProcess = ctx.render_process_;

        // 1. 等待上一帧的 fence 状态
        if (vkWaitForFences(device, 1, &fences_[curFrame_], VK_TRUE, std::numeric_limits<uint64_t>::max()) !=
            VK_SUCCESS) {
            throw std::runtime_error("wait for fence failed");
        }
        vkResetFences(device, 1, &fences_[curFrame_]);

        auto &cmd = cmdBufs_[curFrame_];

        // 2. MVP 矩阵变换
        // trans_mat
        glm::mat4 translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(rect.position, 0.0f));
        // scale_mat
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(rect.size, 1.0f));
        auto modelMat = translateMatrix * scaleMatrix;
        bufferMVPUniformData(modelMat);

        // 3.查询交换链中下一个空 image
        uint32_t imageIndex;
        auto res = vkAcquireNextImageKHR(device, ctx.swapchain_->swapchain,
                                         std::numeric_limits<uint64_t>::max(), imageAvaliableSems_[curFrame_],
                                         VK_NULL_HANDLE,
                                         &imageIndex);

        if (res != VK_SUCCESS) {
            std::cerr << "Render Failed to acquire swap chain image" << std::endl;
            return;
        }

        // 4. 重置 CommandBuffer
        vkResetCommandBuffer(cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

        // 5. 开始记录 CommandBuffer
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        // USAGE_ONE_TIME_SUBMIT_BIT : commandBuffer只执行一次，后续不再使用
        // USAGE_RENDER_PASS_CONTINUE_BIT : commandBuffer在renderPass整个声明周期都要存在
        // USAGE_SIMULTANEOUS_USE_BIT : 可以重复使用 （用于多个线程中同时使用同一个命令缓冲区，commandBuffer不能改变)
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &beginInfo);

        // 6. 开始执行 RenderPass ，通过执行 vkCmdBeginRenderPass
        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderProcess->renderPass_;
        VkRect2D renderAreaExtent{};
        renderAreaExtent.extent = ctx.swapchain_->info.imageExtent;
        renderAreaExtent.offset.x = 0;
        renderAreaExtent.offset.y = 0;
        renderPassBeginInfo.renderArea = renderAreaExtent; // renderPass在屏幕作用位置
        renderPassBeginInfo.framebuffer = ctx.swapchain_->framebuffers[imageIndex];
        VkClearValue clearValue{};
        clearValue.color = {1.0f, 1.0f, 1.0f, 1.0f};
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearValue;

        /*
            VK_SUBPASS_CONTENTS_INLINE :
            主命令缓冲区将直接包含所有必要的绘制和渲染命令,无需使用次级cmd，子通道内容会直接嵌入并记录主cmd
            VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS：
            通道的内容将被记录到一个或多个次级命令缓冲区中,可以创建多个次级命令缓冲区，每个都包含一组特定的渲染命令，然后在需要时从主命令缓冲区中调用它们
        */
        vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        /* 7.
       绑定渲染管线
       使用GPU传入的顶点参数进行渲染 (多个buffer需要进行偏移)
       传入uniform变量 (描述符绑定多个 uniform )
       */

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          renderProcess->pipeline_); // 绑定pipeline
        VkDeviceSize vertexBufferOffset = 0;
        vkCmdBindVertexBuffers(cmd, 0, 1, &deviceVertexBuffer_->buffer_, &vertexBufferOffset);
        vkCmdBindIndexBuffer(cmd, deviceIndicesBuffer_->buffer_, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderProcess->layout_, 0,
                                1, &mvpDescriptorSets_[curFrame_], 0, nullptr);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderProcess->layout_, 1,
                                1, &colorDescriptorSets_[curFrame_], 0, nullptr);


        // 执行 Draw 命令
        // vkCmdDraw(cmd, 3, 1, 0, 0);
        vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);

        // 6. 结束记录 renderPass && CommandBuffer
        vkCmdEndRenderPass(cmd);
        res = vkEndCommandBuffer(cmd);
        if (res != VK_SUCCESS) {
            std::cerr << "Render Failed to end command buffer" << std::endl;
        }

        // 指定提交至GPU 图形队列以及显示队列
        VkSubmitInfo submitGraphicsInfo{};
        submitGraphicsInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitGraphicsInfo.commandBufferCount = 1;
        submitGraphicsInfo.pCommandBuffers = &cmd;
        submitGraphicsInfo.signalSemaphoreCount = 1;
        submitGraphicsInfo.pSignalSemaphores = &renderFinishSems_[curFrame_];
        submitGraphicsInfo.waitSemaphoreCount = 1;
        submitGraphicsInfo.pWaitSemaphores = &imageAvaliableSems_[curFrame_];
        VkPipelineStageFlags flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submitGraphicsInfo.pWaitDstStageMask = &flags;

        res = vkQueueSubmit(ctx.graphicsQueue_, 1, &submitGraphicsInfo, fences_[curFrame_]);
        if (res != VK_SUCCESS) {
            std::cerr << "Render Failed to submit graphics queue res: " << res << std::endl;
            return;
        }

        // 7. 交换数据并提交 GPU
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &ctx.swapchain_->swapchain;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishSems_[curFrame_];

        res = vkQueuePresentKHR(ctx.presentQueue_, &presentInfo);
        if (res != VK_SUCCESS) {
            std::cerr << "Render Failed to present to screen" << std::endl;
            return;
        }

        curFrame_ = (curFrame_ + 1) % maxFlightCount_;
    }

    // 如何创建顶点buffer
    void Renderer::createVertexIndexBuffer() {
        auto &ctx = Context::GetInstance();
        // 创建并填充 Vertex Buffer
        // VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 用于创建 vertex buffer (其他store..uniform...indirect)
        // 一定要设置  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT , 要求内存对宿主机(CPU)可见
        hostVertexBuffer_ = std::make_unique<Buffer>(sizeof(vertices), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                     ctx.device_, ctx.physicalDevice_);

        /*  如果 HOST 不加 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            需要每次 buffer写完调用 vkFlushMappedMemoryRanges 将内存刷新
            每次读取buffer 需要调用  vkInvalidateMappedMemoryRanges 重置*/

        deviceVertexBuffer_ = std::make_unique<Buffer>(sizeof(vertices),
                                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                                       VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                       ctx.device_, ctx.physicalDevice_);
        /* 创建 Indices Buffer */
        hostIndicesBuffer_ = std::make_unique<Buffer>(sizeof(indices),
                                                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                      ctx.device_, ctx.physicalDevice_);

        deviceIndicesBuffer_ = std::make_unique<Buffer>(sizeof(indices),
                                                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                        ctx.device_, ctx.physicalDevice_);

        // copy host buffer -> device buffer
        std::cout << "Renderer create VertexBuffer And IndicesBuffer success" << std::endl;
    }

    // 如何将顶点传入到GPU
    void Renderer::bufferVertexData() {
        uint32_t dataSize = sizeof(vertices);
        memcpy(hostVertexBuffer_->map, vertices.data(), sizeof(vertices));
        // copy hostBuffer -> deviceBuffer
        transformBuffer2Device(*hostVertexBuffer_, *deviceVertexBuffer_, dataSize, 0, 0);

        dataSize = sizeof(indices);
        memcpy(hostIndicesBuffer_->map, indices, dataSize);
        transformBuffer2Device(*hostIndicesBuffer_, *deviceIndicesBuffer_, dataSize, 0, 0);
    }

    /**
     * 使用 buffer 传递uniform对象,每一帧需要 host + deviceLocal buffer
    */
    void Renderer::createUniformBuffers() {
        /* Init Vertex & Indices Buffer （固定图形不需要创建多个）*/
        hostColorUniformBufs.resize(maxFlightCount_);
        localColorUniformBufs_.resize(maxFlightCount_);
        uint64_t uniformSize = sizeof(initColor);
        auto &ctx = Context::GetInstance();

        for (auto &hostUniformBuffer: hostColorUniformBufs) {
            hostUniformBuffer = std::make_unique<Buffer>(uniformSize,
                                                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                         ctx.device_, ctx.physicalDevice_);
        }
        for (auto &deviceUniformBuf: localColorUniformBufs_) {
            deviceUniformBuf = std::make_unique<Buffer>(uniformSize,
                                                        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                        ctx.device_, ctx.physicalDevice_);
        }

        /* Init MVP uniform buffer (MVP 变换每一帧不同，需创建多个)*/
        hostMVPUniformBufs_.resize(maxFlightCount_);
        localMVPUniformBufs_.resize(maxFlightCount_);
        auto mvpSize = sizeof(glm::mat4) * 3;

        for (auto &hostMVPBuf: hostMVPUniformBufs_) {
            hostMVPBuf = std::make_unique<Buffer>(mvpSize,
                                                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                  ctx.device_, ctx.physicalDevice_);
        }

        for (auto &deviceMVPBuf: localMVPUniformBufs_) {
            deviceMVPBuf = std::make_unique<Buffer>(mvpSize,
                                                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                                    VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                                    ctx.device_, ctx.physicalDevice_);

        }

        std::cout << "Renderer create Color And MVP Uniform success" << std::endl;
    }

    void Renderer::SetDrawColor(const Color &color) {
        for (size_t i = 0; i < hostColorUniformBufs.size(); i++) {
            auto &buffer = hostColorUniformBufs[i];

            memcpy(buffer->map, &color, sizeof(color));

            // copy hostUniformBuffer -> deviceUniformBuffer
            transformBuffer2Device(*buffer, *localColorUniformBufs_[i],
                                   sizeof(color), 0, 0);
        }
    }


    void Renderer::transformBuffer2Device(Buffer &src, Buffer &dst, size_t size, size_t srcOffset, size_t dstOffset) {
        auto &ctx = Context::GetInstance();
        auto cmdBuf = ctx.commandManager_->allocateOneCmdBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmdBuf, &beginInfo);

        VkBufferCopy copy{};
        copy.size = size;
        copy.srcOffset = srcOffset;
        copy.dstOffset = dstOffset;
        vkCmdCopyBuffer(cmdBuf, src.buffer_, dst.buffer_, 1, &copy);
        vkEndCommandBuffer(cmdBuf);

        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &cmdBuf;
        auto res = vkQueueSubmit(ctx.graphicsQueue_, 1, &submit, nullptr);
        if (res != VK_SUCCESS) {
            std::cerr << "Render Failed to transformBuffer2Device res: " << res << std::endl;
            return;
        }
        vkDeviceWaitIdle(ctx.device_);
        ctx.commandManager_->FreeCmdBuffer(cmdBuf);
    }

    void Renderer::createDescriptorPool() {
        auto &device = Context::GetInstance().device_;
        VkDescriptorPoolSize poolSizes{};
        poolSizes.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes.descriptorCount = maxFlightCount_;

        /* 1. Init Color DescriptorPool */
        VkDescriptorPoolCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo.maxSets = maxFlightCount_;
        createInfo.poolSizeCount = 1;
        createInfo.pPoolSizes = &poolSizes;
        auto res = vkCreateDescriptorPool(device, &createInfo, nullptr, &colorDescriptorPool_);

        /* 2. Init MVP Vertex DescriptorPool */
        VkDescriptorPoolCreateInfo createInfo2{};
        createInfo2.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo2.maxSets = maxFlightCount_;
        createInfo2.poolSizeCount = 1;
        createInfo2.pPoolSizes = &poolSizes;
        res = vkCreateDescriptorPool(device, &createInfo2, nullptr, &mvpDescriptorPool_);

        if (res != VK_SUCCESS) {
            std::cerr << "Render Failed to create DescriptorPool res: " << res << std::endl;
            return;
        }
    }

    void Renderer::allocateDescriptorSets() {
        auto &ctx = Context::GetInstance();
        auto mvpSetLayout = ctx.shader_->GetDescriptorSetLayouts()[0];
        auto colorSetLayout = ctx.shader_->GetDescriptorSetLayouts()[1];


        // 每一个描述符集都需要一个自己的 setLayout
        std::vector<VkDescriptorSetLayout> colorSetLayouts = std::vector<VkDescriptorSetLayout>(maxFlightCount_,
                                                                                                colorSetLayout);
        std::vector<VkDescriptorSetLayout> mvpSetLayouts = std::vector<VkDescriptorSetLayout>(maxFlightCount_,
                                                                                              mvpSetLayout);
        /* Init MVP DescriptorSet Allocate*/
        VkDescriptorSetAllocateInfo mvpSetAllocateInfo{};
        mvpSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        mvpSetAllocateInfo.descriptorPool = mvpDescriptorPool_;
        mvpSetAllocateInfo.descriptorSetCount = maxFlightCount_;
        mvpSetAllocateInfo.pSetLayouts = mvpSetLayouts.data();
        mvpDescriptorSets_.resize(maxFlightCount_);
        vkAllocateDescriptorSets(ctx.device_, &mvpSetAllocateInfo, mvpDescriptorSets_.data());

        /* Init Color DescriptorSet Allocate*/
        VkDescriptorSetAllocateInfo colorSetAllocateInfo{};
        colorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        colorSetAllocateInfo.descriptorPool = colorDescriptorPool_;
        colorSetAllocateInfo.descriptorSetCount = maxFlightCount_;
        colorSetAllocateInfo.pSetLayouts = colorSetLayouts.data();
        colorDescriptorSets_.resize(maxFlightCount_);
        vkAllocateDescriptorSets(ctx.device_, &colorSetAllocateInfo, colorDescriptorSets_.data());


        std::cout << "Renderer allocate DescriptorSets success" << std::endl;
    }

    // 将 descriptorSets 和 uniformBuffers 绑定
    void Renderer::updateDescriptorSets() {
        auto &ctx = Context::GetInstance();
        for (size_t i = 0; i < colorDescriptorSets_.size(); i++) {
            std::vector<VkWriteDescriptorSet> writes{};
            writes.resize(2);

            auto &mvpSet = mvpDescriptorSets_[i];
            VkDescriptorBufferInfo vertexBufferInfo{};
            vertexBufferInfo.buffer = localMVPUniformBufs_[i]->buffer_;
            vertexBufferInfo.offset = 0;
            vertexBufferInfo.range = localMVPUniformBufs_[i]->buffer_size_;
            VkWriteDescriptorSet vertexWrite{};
            vertexWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            vertexWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            vertexWrite.pBufferInfo = &vertexBufferInfo;
            vertexWrite.dstBinding = 0;
            vertexWrite.dstSet = mvpSet;
            vertexWrite.dstArrayElement = 0; // 绑定uniform数组的哪一个元素 (数组size == 0 则只绑定一个 uniform)
            vertexWrite.descriptorCount = 1;
            writes[0] = vertexWrite;


            auto &set = colorDescriptorSets_[i];
            VkDescriptorBufferInfo colorBufferInfo{};
            colorBufferInfo.buffer = localColorUniformBufs_[i]->buffer_;
            colorBufferInfo.offset = 0;
            colorBufferInfo.range = localColorUniformBufs_[i]->buffer_size_;
            VkWriteDescriptorSet colorWrite{};
            colorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            colorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            colorWrite.pBufferInfo = &colorBufferInfo;
            colorWrite.dstBinding = 0;
            colorWrite.dstSet = set;
            colorWrite.dstArrayElement = 0; // 绑定uniform数组的哪一个元素 (数组size == 0 则只绑定一个 uniform)
            colorWrite.descriptorCount = 1;
            writes[1] = colorWrite;
            vkUpdateDescriptorSets(ctx.device_, writes.size(), writes.data(),
                                   0, nullptr);
        }
    }

    void Renderer::bufferMVPUniformData(const glm::mat4 modelMat) {
        MVP mvp{};
        mvp.project = projectMat_;
        mvp.view = viewMat_;;
        mvp.model = modelMat;

        for (size_t i = 0; i < hostMVPUniformBufs_.size(); i++) {
            auto &buffer = hostMVPUniformBufs_[i];

            if (buffer->map) {
                memcpy(buffer->map, (void *) &mvp, sizeof(mvp));
                // 如果持续渲染，不能 vkUnmapMemory(device, buffer->memory_);
                transformBuffer2Device(*buffer, *localMVPUniformBufs_[i], buffer->buffer_size_, 0, 0);
            }
        }
    }

    void Renderer::initMats() {
        viewMat_ = glm::identity<glm::mat4>();
        projectMat_ = glm::identity<glm::mat4>();
    }

    void Renderer::SetProjectMat(int right, int left, int bottom, int top, int far, int near) {
        projectMat_[0][0] = 2.0f / (right - left);
        projectMat_[1][1] = 2.0f / (top - bottom);
        projectMat_[2][2] = 2.0 / (near - far);
        projectMat_[3][0] = (left + right) / (left - right);
        projectMat_[3][1] = (top + bottom) / (bottom - top);
        projectMat_[3][2] = (near + far) / (far - near);
    }
}