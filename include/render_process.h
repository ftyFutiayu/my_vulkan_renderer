//
// Created by 12381 on 24-7-25.
//
#pragma once

#include "tool.h"
#include "shader.h"


namespace render_2d {
    class RenderProcess final {
    public:
        RenderProcess();

        ~RenderProcess();

        VkPipeline pipeline_;

        void CreatePipeline(int width, int height,VkDevice device);

        void DestroyPipeline(VkDevice device);


    };
}
