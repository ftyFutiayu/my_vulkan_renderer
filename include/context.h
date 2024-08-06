//
// Created by 12381 on 24-7-21.
//
#pragma once

#include <memory>
#include "render_process.h"
#include "tool.h"
#include "swapchain.h"
#include "commandManager.h"

namespace render_2d {
    class Context final {
    public:
        static void Init(const std::vector<const char *> &extensions, CreateSurfaceFunc func);

        static void Quit();

        static Context &GetInstance() {
            assert(context_instance_);
            return *context_instance_;
        }

        ~Context();

        struct QueueFamilyIndices final {
            // 图像操作 命令队列
            std::optional <uint32_t> graphicsQueue;
            // 显示命令队列
            std::optional <uint32_t> presentQueue;

            operator bool() const {
                return graphicsQueue.has_value() && presentQueue.has_value();
            }
        };

        VkInstance instance_;
        VkPhysicalDevice physicalDevice_;
        VkDevice device_;
        VkQueue graphicsQueue_;
        VkQueue presentQueue_;
        QueueFamilyIndices queueFamilyIndices_;
        VkSurfaceKHR surface_;
        std::shared_ptr <SwapChain> swapchain_;
        std::shared_ptr <RenderProcess> render_process_;
        std::shared_ptr <CommandManager> commandManager_;

        void InitSwapChain(int width, int height);

        void InitRenderProcess();

        void InitCommandManager();

        void QuitSwapChain();

        void QuitCommandManager();

    private:
        Context(const std::vector<const char *> &extensions, CreateSurfaceFunc func);

        static std::unique_ptr <Context> context_instance_;

        void createInstance(const std::vector<const char *> &extensions);

        void pickPhysicalDevice();

        void getQueues();

        void createDevice();

        void queryQueueFamilyIndices();
    };
}
