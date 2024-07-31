//
// Created by 12381 on 24-7-21.
//

#pragma  once

#include "context.h"
#include "shader.h"
#include "renderer.h"

namespace render_2d {
    void Init(const std::vector<const char *> &extensions, CreateSurfaceFunc func, int width, int height);

    void Quit();

    inline Renderer &GetRenderer() {
        return *Context::GetInstance().renderer_;
    }
}
