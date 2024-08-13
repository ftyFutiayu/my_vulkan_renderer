#pragma once

#include "tool.h"

namespace render_2d {

    struct Vec {
        static VkVertexInputAttributeDescription GetAttributeDescription();

        static VkVertexInputBindingDescription GetBindingDescription();
    };

    struct Color {
        float r, g, b, a;
    };

    struct Rect {
        glm::vec2 position;
        glm::vec2 size;
    };
}