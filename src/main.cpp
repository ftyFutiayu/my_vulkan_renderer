#define GLFW_INCLUDE_VULKAN

#include "GLFW/glfw3.h"
#include "../include/render2d.h"

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

    glfwMakeContextCurrent(window);
    glClearColor(0.4f, 0.3f, 0.4f, 0.0f);

    /* 获得GLFW 所支持的拓展个数 */
    uint32_t extensionCount = 0;
    const char **extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
    std::vector<const char *> extensionList(extensionCount);
    for (uint32_t i = 0; i < extensionCount; ++i) {
        extensionList[i] = extensions[i];
    }
    for (const auto &extension: extensionList) {
        std::cout << " GLFW Required Extension: " << extension << std::endl;
    }

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
    auto &renderer = render_2d::GetRenderer();
    renderer.Render();
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        /* Draw */
        renderer.Render();
    }

    render_2d::Quit();

    glfwTerminate();
    return 0;
}
