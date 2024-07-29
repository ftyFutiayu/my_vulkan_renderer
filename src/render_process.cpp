//
// Created by 12381 on 24-7-25.
//

#include "../include/render_process.h"

namespace render_2d {
    void RenderProcess::CreatePipeline(int width, int height, VkDevice device) {

        // 图像相关pipeline需要加上 graphics
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        //1. Vertex input
        VkPipelineVertexInputStateCreateInfo inputState{};
        inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        pipelineCreateInfo.pVertexInputState = &inputState;

        //2. Vertex Assembling 指定每个顶点连成的图元
        VkPipelineInputAssemblyStateCreateInfo assemblyStateCreateInfo{};
        assemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        assemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
        assemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineCreateInfo.pInputAssemblyState = &assemblyStateCreateInfo;


        //3. Vertex Shaders && Fragment Shaders
        auto stage = Shader::GetInstance().GetShaderStages();
        pipelineCreateInfo.stageCount = stage.size();
        pipelineCreateInfo.pStages = stage.data();

        // 4.viewport
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        VkViewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(width);
        viewport.height = static_cast<float>(height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport; //视口转换
        viewportState.scissorCount = 1;
        VkRect2D viewportScissor{};
        viewportScissor.offset = {0, 0};
        viewportScissor.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
        viewportState.pScissors = &viewportScissor; // 裁切设置
        pipelineCreateInfo.pViewportState = &viewportState;

        // 5.光栅化
        VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
        rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizerCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizerCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizerCreateInfo.lineWidth = 1.0f; // 边框宽度1
        rasterizerCreateInfo.depthClampEnable = VK_FALSE;
        pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;

        // 6. multi-sampling
        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
        multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // 单个样本
        multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
        pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;

        //7. test depth & stencil

        //8. color blending
        VkPipelineColorBlendStateCreateInfo colorBlendState{};
        colorBlendState.logicOpEnable = VK_FALSE;
        colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendState.attachmentCount = 1; // 1个颜色附件
        VkPipelineColorBlendAttachmentState attachmentState{};
        attachmentState.blendEnable = VK_FALSE; // 禁用color混合
        // 如何往纹理附件输入颜色
        attachmentState.colorWriteMask =
                VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT;

        colorBlendState.pAttachments = &attachmentState; // 定义颜色混合状态
        pipelineCreateInfo.pColorBlendState = &colorBlendState;


        if (vkCreateGraphicsPipelines(device, nullptr, 1,
                                      &pipelineCreateInfo, nullptr, &pipeline_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan device_.");
        }
        std::cout << "Graphics pipeline created successfully." << std::endl;

        //9. layout

        //10. renderPass
    }

    void RenderProcess::DestroyPipeline(VkDevice device) {
        vkDestroyPipeline(device, pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
        std::cout << "Graphics pipeline destroyed successfully." << std::endl;
    }

    RenderProcess::RenderProcess() {

    }

    RenderProcess::~RenderProcess() {

    }

}
