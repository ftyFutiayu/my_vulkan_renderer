//
// Created by 12381 on 24-7-21.
//

#include "render2d.h"

namespace render_2d {
    void Init(const std::vector<const char *> &extensions, CreateSurfaceFunc func) {
        Context::Init(extensions, func);
    }

    void Quit() {
        Context::Quit();
    }
}