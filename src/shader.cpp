//
// Created by 12381 on 24-7-25.
//

#include "../include/shader.h"

namespace render_2d {
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

        initDescriptorSetLayouts();
    }


    Shader::~Shader() {
        for (auto &setLayout: setLayouts_) {
            vkDestroyDescriptorSetLayout(device_, setLayout, nullptr);
        }
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


    void Shader::initDescriptorSetLayouts() {
        setLayouts_.resize(2);
        /* Init VertexShader MVP Uniform Set = 0 */
        VkDescriptorSetLayoutBinding vertexLayoutBinding{};
        vertexLayoutBinding.binding = 0;
        vertexLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        vertexLayoutBinding.descriptorCount = 1;
        vertexLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo vertexLayoutCreateInfo{};
        vertexLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        vertexLayoutCreateInfo.bindingCount = 1;
        vertexLayoutCreateInfo.pBindings = &vertexLayoutBinding;
        VkDescriptorSetLayout vertexSetLayout{};
        vkCreateDescriptorSetLayout(device_, &vertexLayoutCreateInfo, nullptr, &vertexSetLayout);
        setLayouts_[0] = vertexSetLayout;


        /* Init FragmentShader Color Uniform Set = 1*/
        VkDescriptorSetLayoutBinding fragLayoutBinding{};
        fragLayoutBinding.binding = 0;
        fragLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        fragLayoutBinding.descriptorCount = 1;
        fragLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo fragLayoutCreateInfo{};
        fragLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        fragLayoutCreateInfo.bindingCount = 1;
        fragLayoutCreateInfo.pBindings = &fragLayoutBinding;
        VkDescriptorSetLayout fragmentSetLayout{};
        vkCreateDescriptorSetLayout(device_, &fragLayoutCreateInfo, nullptr, &fragmentSetLayout);
        setLayouts_[1] = fragmentSetLayout;


        std::cout << "DescriptorSet layouts initialized successfully. setLayout size -> " << setLayouts_.size()
                  << std::endl;
    }
}

