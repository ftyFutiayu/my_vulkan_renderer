#include "../include/buffer.h"

namespace render_2d {
    Buffer::Buffer(uint64_t buffer_size, VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags property,
                   VkDevice device, VkPhysicalDevice gpu) : device_(device), gpu_(gpu) {
        buffer_size_ = buffer_size;
        createBuffer(buffer_size, bufferUsage);
        auto info = queryMemoryInfo(buffer_, property);
        allocateMemory(info);
        bindingMemory();

        if (property & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            vkMapMemory(device_, memory_, 0, buffer_size, 0, &map);
        } else {
            map = nullptr;
        }
    }

    Buffer::~Buffer() {
        if (map) {
            vkUnmapMemory(device_, memory_);
        }

        vkDestroyBuffer(device_, buffer_, nullptr);
        vkFreeMemory(device_, memory_, nullptr);
    }

    // 创建 vkBuffer
    void Buffer::createBuffer(uint64_t size, VkBufferUsageFlags bufferUsage) {
        VkBufferCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.size = size;
        createInfo.usage = bufferUsage;                     // 当前buffer的用法
        createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // 队列独占模式
        vkCreateBuffer(device_, &createInfo, nullptr, &buffer_);
    }

    // 开辟对应大小的内存
    void Buffer::allocateMemory(MemoryInfo info) {
        VkMemoryAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.allocationSize = info.size;
        allocateInfo.memoryTypeIndex = info.index;
        vkAllocateMemory(device_, &allocateInfo, nullptr, &memory_);
    }

    // 将 vkBuffer绑定对应内存上
    void Buffer::bindingMemory() {
        vkBindBufferMemory(device_, buffer_, memory_, 0);
    }

    // 查询 buffer在显存中的信息
    Buffer::MemoryInfo Buffer::queryMemoryInfo(VkBuffer buffer, VkMemoryPropertyFlags property) {
        MemoryInfo info;
        VkMemoryRequirements requirement{};
        vkGetBufferMemoryRequirements(device_, buffer, &requirement);
        info.size = requirement.size;

        VkPhysicalDeviceMemoryProperties memoryProperties;
        vkGetPhysicalDeviceMemoryProperties(gpu_, &memoryProperties);

        for (size_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
            if ((1 << i) && requirement.memoryTypeBits || memoryProperties.memoryTypes[i].propertyFlags & property) {
                info.index = i;
            }
        }
        return info;
    }

}