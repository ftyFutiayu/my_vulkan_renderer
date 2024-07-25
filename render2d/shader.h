//
// Created by 12381 on 24-7-25.
//

#pragma once

#include "tool.h"
#include "context.h"

namespace render_2d {
    class Shader final {
    public:
        static void Init(const std::string &vertex_source, const std::string &fragment_source);

        static void Quit();

        // 正常Renderer 不使用单例模式
        static Shader &GetInstance() {
            return *shader_;
        }

        VkShaderModule vertexShaderModule_;
        VkShaderModule fragmentShaderModule_;

        ~Shader();

    private:
        Shader(const std::string &vertex_source, const std::string &fragment_source);

        static std::unique_ptr<Shader> shader_;
    };

}
