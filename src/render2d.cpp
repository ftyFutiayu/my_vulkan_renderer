//
// Created by 12381 on 24-7-21.
//

#include "../include/render2d.h"

namespace render_2d {
    std::unique_ptr<Renderer> renderer_;

    void Init(const std::vector<const char *> &extensions, CreateSurfaceFunc func, int width, int height) {
        Context::Init(extensions, func);
        auto &ctx = Context::GetInstance();
        ctx.InitShaderModules();
        ctx.InitSwapChain(width, height);
        ctx.InitRenderProcess();
        ctx.swapchain_->CreateFramebuffers(width, height);
        ctx.InitCommandManager();

        // init vulkan Renderer
        renderer_ = std::make_unique<Renderer>(Context::GetInstance().swapchain_->images.size());
        // renderer_ = std::make_unique<Renderer>(1);
        renderer_->SetProjectMat(width, 0, 0, height, -1, 1);
    }

    void Quit() {
        // 等待GPU所有操作完成后释放资源
        auto &ctx = Context::GetInstance();
        vkDeviceWaitIdle(ctx.device_);
        renderer_.reset();
        ctx.render_process_.reset();
        ctx.QuitSwapChain();
        ctx.QuitCommandManager();
        ctx.QuitShaderModules();
        Context::Quit();
    }

    Renderer *GetRenderer() {
        return renderer_.get();
    }
}