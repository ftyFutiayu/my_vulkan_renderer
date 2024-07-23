//
// Created by 12381 on 24-7-21.
//
#pragma once

#include "vulkan/vulkan.h"
#include <memory>
#include <cassert>
#include <optional>

namespace render_2d {
    class Context final {
    public:
        static void Init();

        static void Quit();

        static Context &GetInstance() {
            assert(context_instance_);
            return *context_instance_;
        }


        ~Context();

        struct QueueFamilyIndices final {
            // 图像操作用的命令队列
            std::optional<uint32_t> graphicsQueue;
        };

        VkInstance instance_;
        VkPhysicalDevice physicalDevice_;
        VkDevice device_;
        VkQueue graphicsQueue_;
        QueueFamilyIndices queueFamilyIndices_;
    private:
        Context();

        static std::unique_ptr<Context> context_instance_;

        void createInstance();

        void pickPhysicalDevice();

        void getQueues();

        void createDevice();

        void queryQueueFamilyIndices();
    };
}

