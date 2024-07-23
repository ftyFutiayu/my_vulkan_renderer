#include "context.h"
#include <vector>
#include <iostream>
#include <cstring>

namespace render_2d {

    std::unique_ptr<Context> Context::context_instance_ = nullptr;

    void Context::Init() {
        context_instance_.reset(new Context);
    }

    void Context::Quit() {
        context_instance_.reset();
    }

    Context::Context() {
        createInstance();
        pickPhysicalDevice();
        queryQueueFamilyIndices();
        createDevice();
        getQueues();
    }

    Context::~Context() {
        // 需要按照顺序销毁
        vkDestroyDevice(device_, nullptr);
        vkDestroyInstance(instance_, nullptr);
    }

    void Context::createInstance() {
        VkInstanceCreateInfo createInfo{};
        VkApplicationInfo appInfo{};
        appInfo.apiVersion = VK_API_VERSION_1_3;
        createInfo.pApplicationInfo = &appInfo;

        // 请求的验证层
        std::vector<const char *> requestedLayers = {"VK_LAYER_KHRONOS_validation"};

        // 获取所有可用的层
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        // 检查请求的层是否可用
        std::vector<const char *> enabledLayers;
        for (const auto &layerName: requestedLayers) {
            for (const auto &layerProps: availableLayers) {
                if (strcmp(layerProps.layerName, layerName) == 0) {
                    enabledLayers.push_back(layerName);
                    break;
                }
            }
        }

        // 如果任何请求的层不可用，则抛出异常
        if (enabledLayers.size() != requestedLayers.size()) {
            throw std::runtime_error("One or more requested validation layers are not available.");
        }

        // 设置创建信息中的层
        createInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
        createInfo.ppEnabledLayerNames = enabledLayers.data();

        // 打印所有可用的层
        std::cout << "Available Vulkan layers:" << std::endl;
        for (const auto &layerProps: availableLayers) {
            std::cerr << "  " << layerProps.layerName << std::endl;
        }

        // 创建Vulkan实例
        if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance_");
        }

        std::cout << "Vulkan instance_ created" << std::endl;
    }


    void Context::pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());
        if (deviceCount == 0) {
            throw std::runtime_error("No suitable GPU found");
        }
        for (auto device: devices) {
            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(device, &features);
//            if (features.geometryShader) {
//                physicalDevice_ = device_;
//                break;
//            }
        }
        physicalDevice_ = devices[0];
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevice_, &properties);
        std::cout << "Using GPU: " << properties.deviceName << std::endl;
    }

    /*
     * 逻辑设备和GPU交互，不能直接使用 物理设备
     */
    void Context::createDevice() {
        // Queue  《===device_======》physicalDevice_ 传输 commandBuffer作为桥梁

        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        // 队列大小
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.queueFamilyIndex = *queueFamilyIndices_.graphicsQueue;

        // logicDevice 可以设置拓展和层 extensions
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        if (vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan device_.");
        }
        std::cout << "Vulkan device_ created" << std::endl;
    }

    // 查询当前物理设备支持的队列属性
    void Context::queryQueueFamilyIndices() {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, queueFamilies.data());

        for (int i = 0; i < queueFamilyCount; i++) {
            auto queueFamily = queueFamilies[i];
            // 最多支持创建几个队列
            queueFamily.queueCount;

            // 支持图像操作的queueFamilyIndex
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queueFamilyIndices_.graphicsQueue = i;
                break;
            }
        }
    }

    void Context::getQueues() {
        vkGetDeviceQueue(device_, *queueFamilyIndices_.graphicsQueue, 0, &graphicsQueue_);
        std::cout << "Get graphics queue" << std::endl;
    }

}
