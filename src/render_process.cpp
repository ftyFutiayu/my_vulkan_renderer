//
// Created by 12381 on 24-7-25.
//

#include "../include/render_process.h"

namespace render_2d {
    RenderProcess::RenderProcess(VkDevice &device, SwapChain &swapchain, Shader &shader) : device_(device),
                                                                                           swapchain_(swapchain) {
        initLayout(shader);
        initRenderPass();
        createPipeline(shader);
        std::cout << "Initializing Render Process...\n";
    }

    RenderProcess::~RenderProcess() {
        vkDestroyRenderPass(device_, renderPass_, nullptr);
        vkDestroyPipelineLayout(device_, layout_, nullptr);
        vkDestroyPipeline(device_, pipeline_, nullptr);
        pipeline_ = VK_NULL_HANDLE;
        std::cout << "Graphics pipeline destroyed successfully." << std::endl;
    }

    void RenderProcess::createPipeline(Shader &shader) {

        // 图像相关pipeline需要加上 graphics
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        // 1. Vertex input
        VkPipelineVertexInputStateCreateInfo inputState{};
        inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        auto attribute = Vec::GetAttributeDescription();
        auto binding = Vec::GetBindingDescription();
        inputState.vertexAttributeDescriptionCount = 1;
        inputState.vertexBindingDescriptionCount = 1;
        inputState.pVertexAttributeDescriptions = &attribute;
        inputState.pVertexBindingDescriptions = &binding;
        pipelineCreateInfo.pVertexInputState = &inputState;

        // 2. Vertex Assembling 指定每个顶点连成的图元
        VkPipelineInputAssemblyStateCreateInfo assemblyStateCreateInfo{};
        assemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        assemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
        assemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineCreateInfo.pInputAssemblyState = &assemblyStateCreateInfo;

        // 3. Vertex Shaders && Fragment Shaders
        auto stage = shader.GetShaderStages(); // get vertex and fragment shader VkPipelineShaderStageCreateInfo
        pipelineCreateInfo.stageCount = stage.size();
        pipelineCreateInfo.pStages = stage.data();

        // 4.viewport
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        VkViewport viewport;
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapchain_.info.imageExtent.width);
        viewport.height = static_cast<float>(swapchain_.info.imageExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport; // 视口转换
        viewportState.scissorCount = 1;
        VkRect2D viewportScissor{};
        viewportScissor.offset = {0, 0};
        viewportScissor.extent = swapchain_.info.imageExtent;
        viewportState.pScissors = &viewportScissor; // 裁切设置
        pipelineCreateInfo.pViewportState = &viewportState;

        // 5.光栅化
        VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo{};
        rasterizerCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizerCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizerCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
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

        // 7. test depth & stencil

        // 8. color blending
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

        // 9. layout
        pipelineCreateInfo.layout = layout_;

        // 10. renderPass
        pipelineCreateInfo.renderPass = renderPass_;

        auto res = vkCreateGraphicsPipelines(device_, nullptr, 1,
                                             &pipelineCreateInfo, nullptr, &pipeline_);
        // create pipeline
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Render pipeline");
        }
        std::cout << "Graphics pipeline created successfully." << std::endl;
    }

    // 初始化 Layout，和uniform数据在shader中布局
    void RenderProcess::initLayout(Shader &shader) {
        VkPipelineLayoutCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        auto setLayouts = shader.GetDescriptorSetLayouts();
        createInfo.setLayoutCount = setLayouts.size();
        createInfo.pSetLayouts = setLayouts.data();
        vkCreatePipelineLayout(device_, &createInfo, nullptr, &layout_);
        std::cout << "Pipeline layout created Success" << std::endl;
    }

    /**
     * 创建 render_pass
     * const VkAttachmentDescription*    pAttachments;
     * const VkSubpassDescription*       pSubpasses;
     * const VkSubpassDependency*        pDependencies;
     */
    void RenderProcess::initRenderPass() {

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        // 1.VkAttachmentDescription (纹路附件描述)(只有颜色附件，所以只需要一个 )
        VkAttachmentDescription attachmentDescription;
        attachmentDescription.format = swapchain_.info.format.format;
        attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        // 加载的时候全部清空
        attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // 存储到显存
        attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
        // 模板缓冲
        /*
        frameBuffer为多个 attachment的绑定组
        一个 frameBuffer为多个 需要：
        1. 至少一个颜色附件，填入颜色信息 (attachment就是图像)
        2. 0个或者多个深度/模板缓冲，填入深度/模板信息 （3维中，用zbuffer描述物体前后顺序）
        */
        attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;

        // 2. VkSubpassDescription (子pass描述)
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentRef.attachment = 0; // 第0个颜色附件
        VkSubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorAttachmentRef;

        // 3. SubpassDependency (多个subpass需要指定执行顺序，vulkan自带initSubpass)
        VkSubpassDependency dependency{};
        // src : 先执行的 subpass / dst : 后执行的 subpass
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // 外部子pass
        // 0 同VkAttachmentReference，为设置VkAttachmentDescription数组的下标，指定使用哪个纹理附件
        dependency.dstSubpass = 0;
        // 当前渲染通道如何修改权限 （修改颜色的等...）
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        // stages : 当前渲染通道走完以后应用到什么场景中 (均设置为颜色输出)
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &attachmentDescription;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpassDescription;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(device_, &renderPassInfo, nullptr, &renderPass_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan render pass.");
        }
        std::cout << "Render pass created successfully." << std::endl;
    }
}
