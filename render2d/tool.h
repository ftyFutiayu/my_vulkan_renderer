//
// Created by 12381 on 24-7-23.
//

#include "vulkan/vulkan.h"
#include <memory>
#include <cassert>
#include <optional>
#include <vector>
#include <iostream>
#include <cstring>
#include <functional>


namespace render_2d {
    using CreateSurfaceFunc = std::function<VkSurfaceKHR(VkInstance)>;

}

