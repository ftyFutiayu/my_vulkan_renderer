//
// Created by 12381 on 24-7-25.
//

#pragma once

#include "tool.h"

namespace render_2d {
    class Shader final {
    public:
        Shader(const std::string &vertex_source, const std::string &fragment_source, VkDevice &device);

        ~Shader();

        std::vector<VkPipelineShaderStageCreateInfo> GetShaderStages();

        const std::vector<VkDescriptorSetLayout> &GetDescriptorSetLayouts() const { return setLayouts_; }

    private:
        std::vector<VkDescriptorSetLayout> setLayouts_;

        std::vector<VkPipelineShaderStageCreateInfo> stages_;

        void initStages();

        void initDescriptorSetLayouts();

        VkShaderModule vertexShaderModule_;

        VkShaderModule fragmentShaderModule_;

    private:
        VkDevice device_;
    };

}
