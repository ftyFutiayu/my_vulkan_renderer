#include "../include/vertex.h"


namespace render_2d {
    // 顶点数据具体属性，位置、颜色、法线、纹理坐标等
    VkVertexInputAttributeDescription Vec::GetAttributeDescription() {
        VkVertexInputAttributeDescription attributeDescription{};
        attributeDescription.binding = 0;                      // 顶点数据在缓冲区中的 binding 绑定点
        attributeDescription.format = VK_FORMAT_R32G32_SFLOAT; // 位置属性的数据格式
        attributeDescription.location = 0;                     // 位置属性在 shader 里的位置
        attributeDescription.offset = 0;                       // 位置属性在 Vertex 结构体中的偏移量 （多组顶点需要设置offset）
        return attributeDescription;
    }

    // 顶点数据如何读取数据以及内存布局
    VkVertexInputBindingDescription Vec::GetBindingDescription() {
        VkVertexInputBindingDescription description{};
        description.binding = 0;                             // 顶点数据在缓冲区中的 binding 绑定点
        description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // 顶点数据按顶点为 也可以设置每个图元传输
        description.stride = (sizeof(float) * 2);                 // 顶点数据在缓冲区中的步长
        return description;
    }
}
