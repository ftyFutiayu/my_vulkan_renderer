//
// Created by 12381 on 24-7-25.
//

#include "../include/shader.h"

namespace render_2d {
    std::unique_ptr<Shader> Shader::shader_ = nullptr;

    void Shader::Init(const std::string &vertex_source, const std::string &fragment_source, VkDevice &device) {
        shader_.reset(new Shader(vertex_source, fragment_source, device));
    }

    void Shader::Quit() {
        shader_.reset();
    }

    Shader::Shader(const std::string &vertex_source, const std::string &fragment_source, VkDevice &device) {
        device_ = device;
        VkShaderModuleCreateInfo vertexShaderCreateInfo{};
        vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vertexShaderCreateInfo.codeSize = vertex_source.size();
        vertexShaderCreateInfo.pCode = (uint32_t *) vertex_source.data();
        vkCreateShaderModule(device, &vertexShaderCreateInfo,
                             nullptr, &vertexShaderModule_);
        std::cout << "Vertex shader module created successfully." << std::endl;

        VkShaderModuleCreateInfo fragmentShaderCreateInfo{};
        fragmentShaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        fragmentShaderCreateInfo.codeSize = fragment_source.size();
        fragmentShaderCreateInfo.pCode = (uint32_t *) fragment_source.data();
        vkCreateShaderModule(device_, &fragmentShaderCreateInfo,
                             nullptr, &fragmentShaderModule_);
        std::cout << "Fragment shader module created successfully." << std::endl;

        initStages();
    }


    Shader::~Shader() {
        vkDestroyShaderModule(device_, fragmentShaderModule_, nullptr);
        vkDestroyShaderModule(device_, vertexShaderModule_, nullptr);
        std::cout << "Shader module destroyed successfully." << std::endl;
    }


    std::vector<VkPipelineShaderStageCreateInfo> Shader::GetShaderStages() {
        return stages_;
    }

    void Shader::initStages() {
        stages_.resize(2);
        VkPipelineShaderStageCreateInfo vertexStage{};
        vertexStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexStage.module = vertexShaderModule_;
        vertexStage.pName = "main";
        stages_[0] = vertexStage;

        VkPipelineShaderStageCreateInfo fragmentStage{};
        fragmentStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentStage.module = fragmentShaderModule_;
        fragmentStage.pName = "main";
        stages_[1] = fragmentStage;
        std::cout << "Shader stages initialized successfully." << std::endl;
    }

}

