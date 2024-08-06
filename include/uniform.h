#pragma once

#include "tool.h"

namespace render_2d {
    struct Color final {
        float r, g, b, a;
    };

    struct uniform final {
        Color color;

        static VkDescriptorSetLayoutBinding GetLayoutBinding() {
            VkDescriptorSetLayoutBinding layoutBinding{};

            layoutBinding.binding = 0;
            layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            layoutBinding.descriptorCount = 1;
            return layoutBinding;
        }
    };
}  // namespace render_2d