//
// Created by 12381 on 24-7-21.
//

#pragma  once

#include "context.h"

namespace render_2d {
    void Init(const std::vector<const char *> &extensions, CreateSurfaceFunc func);

    void Quit();
}