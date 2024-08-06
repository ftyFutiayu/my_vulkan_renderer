//
// Created by 12381 on 24-7-21.
//

#include "../include/render2d.h"

namespace render_2d {
    std::unique_ptr <Renderer> renderer_;

    void Init(const std::vector<const char *> &extensions, CreateSurfaceFunc func, int width, int height) {
        Context::Init(extensions, func);
        Context::GetInstance().InitSwapChain(width, height);
        auto logic_device = Context::GetInstance().device_;
        Shader::Init(ReadWholeFile("../vert.spv"),
                     ReadWholeFile("../frag.spv"),
                     logic_device);
        Context::GetInstance().InitRenderProcess();

        auto render_process_ = Context::GetInstance().render_process_;
        render_process_->InitRenderPass();
        render_process_->InitLayout();
        render_process_->CreatePipeline(width, height);
        Context::GetInstance().swapchain_->CreateFramebuffers(width, height);
        Context::GetInstance().InitCommandManager();

        // init vulkan Renderer
        renderer_ = std::make_unique<Renderer>(logic_device,
                                               Context::GetInstance().physicalDevice_,
                                               Context::GetInstance().swapchain_,
                                               render_process_,
                                               Context::GetInstance().graphicsQueue_,
                                               Context::GetInstance().presentQueue_,
                                               Context::GetInstance().commandManager_);
    }

    void Quit() {
        // 等待GPU所有操作完成后释放资源
        vkDeviceWaitIdle(Context::GetInstance().device_);
        renderer_.reset();
        Context::GetInstance().render_process_.reset();
        Context::GetInstance().QuitSwapChain();
        Context::GetInstance().QuitCommandManager();
        Shader::Quit();
        Context::Quit();
    }

    Renderer *GetRenderer() {
        return renderer_.get();
    }
}