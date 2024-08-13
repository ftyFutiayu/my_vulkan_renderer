//
// Created by 12381 on 24-7-23.
//
#pragma once

#include "vulkan/vulkan.h"
#include <memory>
#include <cassert>
#include <optional>
#include <vector>
#include <iostream>
#include <cstring>
#include <functional>
#include <fstream>
#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"


namespace render_2d {
    using CreateSurfaceFunc = std::function<VkSurfaceKHR(VkInstance)>;

    std::string ReadWholeFile(const std::string &filename);
}

