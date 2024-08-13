#pragma once

#include <utility>
#include "context.h"
#include "buffer.h"

namespace render_2d {
    class Renderer final {
    public:
        Renderer(int maxFlightCount);

        ~Renderer();

        void DrawRect(const Rect &rect);

        void SetDrawColor(const Color &color);

        void SetProjectMat(int right, int left, int bottom, int top, int far, int near);

    private:
        struct MVP {
            glm::mat4 project;
            glm::mat4 view;
            glm::mat4 model;
        };

        std::vector<VkFence> fences_;

        std::vector<VkSemaphore> imageAvaliableSems_;

        std::vector<VkSemaphore> renderFinishSems_;

        std::vector<VkCommandBuffer> cmdBufs_;

        int maxFlightCount_;

        int curFrame_ = 0;

        std::unique_ptr<Buffer> hostVertexBuffer_; // 使用CPU&GPU共享IO内存 buffer

        std::unique_ptr<Buffer> deviceVertexBuffer_; // GPU独占的buffer

        std::unique_ptr<Buffer> hostIndicesBuffer_; // 顶点索引buffer

        std::unique_ptr<Buffer> deviceIndicesBuffer_; // GPU独占的顶点索引buffer

        std::vector<std::unique_ptr<Buffer>> hostColorUniformBufs; // GPU独占的uniform buffer

        std::vector<std::unique_ptr<Buffer>> localColorUniformBufs_; // GPU独占的

        std::vector<std::unique_ptr<Buffer>> hostMVPUniformBufs_;

        std::vector<std::unique_ptr<Buffer>> localMVPUniformBufs_;

        VkDescriptorPool colorDescriptorPool_;

        VkDescriptorPool mvpDescriptorPool_;

        std::vector<VkDescriptorSet> colorDescriptorSets_;

        std::vector<VkDescriptorSet> mvpDescriptorSets_;

        void createFences();

        void createSemaphores();

        void createCmdBuffers();

        void createVertexIndexBuffer();

        void bufferVertexData();

        void createUniformBuffers();

        void transformBuffer2Device(Buffer &src, Buffer &dst, size_t size, size_t srcOffset, size_t dstOffset);

        void createDescriptorPool();

        void allocateDescriptorSets();

        void updateDescriptorSets();

        void initMats();

        void bufferMVPUniformData(const glm::mat4 modelMat);

        glm::mat4 projectMat_;

        glm::mat4 viewMat_;
    };
}
