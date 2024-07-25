//
// Created by 12381 on 24-7-25.
//

#include "shader.h"

namespace render_2d {
    std::unique_ptr<Shader> Shader::shader_ = nullptr;

    void Shader::Init(const std::string &vertex_source, const std::string &fragment_source) {
        shader_.reset(new Shader(vertex_source, fragment_source));
    }

    void Shader::Quit() {
        shader_.reset();
    }

    Shader::Shader(const std::string &vertex_source, const std::string &fragment_source) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = vertex_source.size();
        createInfo.pCode = (uint32_t *) vertex_source.data();
        vkCreateShaderModule(Context::GetInstance().device_, &createInfo,
                             nullptr, &vertexShaderModule_);
        std::cout << "Vertex shader module created successfully." << std::endl;

        createInfo.codeSize = fragment_source.size();
        createInfo.pCode = (uint32_t *) fragment_source.data();
        vkCreateShaderModule(Context::GetInstance().device_, &createInfo,
                             nullptr, &fragmentShaderModule_);
        std::cout << "Fragment shader module created successfully." << std::endl;
    }


    Shader::~Shader() {
        auto &device = Context::GetInstance().device_;
        vkDestroyShaderModule(device, fragmentShaderModule_, nullptr);
        vkDestroyShaderModule(device, vertexShaderModule_, nullptr);
        std::cout << "Shader module destroyed successfully." << std::endl;
    }

}

