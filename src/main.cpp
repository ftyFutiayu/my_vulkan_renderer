#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"
#include "../include/render2d.h"

// 全局变量，用于跟踪当前选择的颜色  
render_2d::Color currentColor = {1.0f, 0.0f, 0.0f, 1.0f}; // 初始为红色  

float x = 100.0f;
float y = 100.0f;

// 键盘事件处理函数  
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_1:
                currentColor = {1.0f, 0.0f, 0.0f, 1.0f}; // 红色  
                break;
            case GLFW_KEY_2:
                currentColor = {0.0f, 1.0f, 0.0f, 1.0f}; // 绿色  
                break;
            case GLFW_KEY_3:
                currentColor = {0.0f, 0.0f, 1.0f, 1.0f}; // 蓝色  
                break;
            case GLFW_KEY_4:
                currentColor = {1.0f, 1.0f, 0.0f, 1.0f}; // 黄色  
                break;
            case GLFW_KEY_5:
                currentColor = {1.0f, 0.0f, 1.0f, 1.0f}; // 紫色  
                break;
            case GLFW_KEY_W:
            case GLFW_KEY_UP:
                y -= 10;
                break;
            case GLFW_KEY_S:
            case GLFW_KEY_DOWN:
                y += 10;
                break;
            case GLFW_KEY_A:
            case GLFW_KEY_LEFT:
                x -= 10;
                break;
            case GLFW_KEY_D:
            case GLFW_KEY_RIGHT:
                x += 10;
                break;
            default:
                break;
        }
    }
}

int main(void) {
    /* Initialize the library */
    if (!glfwInit())
        return -1;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    /* Create a windowed mode window */
    GLFWwindow *window = glfwCreateWindow(1024, 720, "GLFW CMake starter", NULL, NULL);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* 获得GLFW 所支持的拓展个数 */
    uint32_t extensionCount = 0;
    const char **extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
    std::vector<const char *> extensionList(extensionCount);
    for (uint32_t i = 0; i < extensionCount; ++i) {
        extensionList[i] = extensions[i];
    }
    extensionList.emplace_back("VK_EXT_debug_utils");

    render_2d::Init(extensionList, [&](VkInstance instance) {
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            const char *errorDescription;
            glfwGetError(&errorDescription);
            std::cerr << "GLFW Error  " << errorDescription << std::endl;
            throw std::runtime_error("Failed to create GLFW window surface!");
        }
        return surface;
    }, 1024, 720);

    /* Get Vulkan Renderer */
    auto renderer = render_2d::GetRenderer();

    /* 注册键盘事件处理函数 */
    glfwSetKeyCallback(window, keyCallback);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        /* 根据键盘数字输入改变颜色*/
        // 接收数字1：
        renderer->SetDrawColor(currentColor);


        /* Draw */
        renderer->DrawRect(render_2d::Rect{glm::vec2(x, y), glm::vec2(200, 300)});
    }

    render_2d::Quit();

    glfwTerminate();
    return 0;
}
