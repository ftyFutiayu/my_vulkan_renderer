//
// Created by 12381 on 24-7-21.
//

#include "render2d.h"


namespace render_2d {
    void Init(const std::vector<const char *> &extensions, CreateSurfaceFunc func, int width, int height) {
        Context::Init(extensions, func);
        Context::GetInstance().InitSwapChain(width, height);

        Shader::Init(ReadWholeFile("../vert.spv"), ReadWholeFile("../frag.spv"));
    }

    void Quit() {
        Context::GetInstance().QuitSwapChain();
        Shader::Quit();
        Context::Quit();
    }
}