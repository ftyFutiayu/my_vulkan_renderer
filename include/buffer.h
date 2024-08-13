#pragma once

#include "tool.h"

namespace render_2d {
    class Buffer final {

    public:
        VkBuffer buffer_;

        VkDeviceMemory memory_;

        uint64_t buffer_size_;

        Buffer(uint64_t buffer_size, VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags property,
               VkDevice device, VkPhysicalDevice gpu);

        ~Buffer();

        void *map; // 如果host可见，使用 map 进行数据操作
    private:
        // 查询内存信息 用于内存分配指定内存索引
        struct MemoryInfo {
            size_t size;
            uint32_t index;
        };

        void createBuffer(uint64_t size, VkBufferUsageFlags bufferUsage);

        void allocateMemory(MemoryInfo info);

        void bindingMemory();

        MemoryInfo queryMemoryInfo(VkBuffer buffer, VkMemoryPropertyFlags property);

    private:
        VkDevice device_;

        VkPhysicalDevice gpu_;
    };

}