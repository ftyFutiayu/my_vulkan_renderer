//
// Created by 12381 on 24-7-21.
//

#include "../include/render2d.h"


namespace render_2d {
    void Init(const std::vector<const char *> &extensions, CreateSurfaceFunc func, int width, int height) {
        Context::Init(extensions, func);
        Context::GetInstance().InitSwapChain(width, height);
        auto logic_device = Context::GetInstance().device_;
        Shader::Init(ReadWholeFile("../vert.spv"),
                     ReadWholeFile("../frag.spv"),
                     logic_device);
        Context::GetInstance().render_process_->CreatePipeline(width, height, logic_device);
    }

    void Quit() {
        Context::GetInstance().render_process_->DestroyPipeline(Context::GetInstance().device_);
        Context::GetInstance().QuitSwapChain();
        Shader::Quit();
        Context::Quit();
    }
}