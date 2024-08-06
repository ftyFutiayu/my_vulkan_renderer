#include "../include/context.h"

namespace render_2d {
    std::unique_ptr <Context> Context::context_instance_ = nullptr;

    void Context::Init(const std::vector<const char *> &extensions, CreateSurfaceFunc func) {
        context_instance_.reset(new Context(extensions, std::move(func)));
    }

    void Context::Quit() {
        context_instance_.reset();
    }

    Context::Context(const std::vector<const char *> &extensions, CreateSurfaceFunc func) {
        createInstance(extensions);
        pickPhysicalDevice();
        // 获取glfw的surface
        surface_ = func(instance_);
        queryQueueFamilyIndices();
        createDevice();
        getQueues();
    }

    Context::~Context() {
        std::cout << "Destroying Vulkan context" << std::endl;
        // 需要按照Create 的反顺序销毁
        vkDestroySurfaceKHR(instance_, surface_, nullptr);
        vkDestroyDevice(device_, nullptr);
        vkDestroyInstance(instance_, nullptr);
    }

    void Context::createInstance(const std::vector<const char *> &extensions) {
        VkInstanceCreateInfo createInfo{};
        VkApplicationInfo appInfo{};
        // ubuntu 不支持 1.3
        appInfo.apiVersion = VK_API_VERSION_1_0;
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // 打印请求的扩展名
        std::cout << "CreateInstance Requested Vulkan extensions:" << std::endl;
        for (const auto &ext: extensions) {
            std::cout << "  " << ext << std::endl;
        }

        // 请求的验证层
        std::vector<const char *> requestedLayers = {"VK_LAYER_KHRONOS_validation"};

        // 获取所有可用的层
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector <VkLayerProperties> availableLayers(layerCount);
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

        // 如果任何请求的层不可用，则抛出异常throwdLayers.size());
        createInfo.ppEnabledLayerNames = enabledLayers.data();

        // 打印所有可用的层
        std::cout << "Available Vulkan layers:" << std::endl;
        for (const auto &layerProps: availableLayers) {
            std::cerr << "  " << layerProps.layerName << std::endl;
        }

        // 创建Vulkan实例
        if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan instance");
        }

        std::cout << "Vulkan instance created" << std::endl;

        // 验证创建的实例是否启用了正确的扩展
        uint32_t extensionCount;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector <VkExtensionProperties> enabledExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, enabledExtensions.data());

        std::cout << "CreateInstance Enabled Vulkan extensions:" << std::endl;
        for (const auto &extProp: enabledExtensions) {
            std::cout << "  " << extProp.extensionName << std::endl;
        }
    }

    void Context::pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
        std::vector <VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());
        if (deviceCount == 0) {
            throw std::runtime_error("No suitable GPU found");
        }
        for (auto device: devices) {
            VkPhysicalDeviceFeatures features;
            vkGetPhysicalDeviceFeatures(device, &features);
            if (features.geometryShader) {
                physicalDevice_ = device;
                break;
            }
        }
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevice_, &properties);
        std::cout << "Using GPU: " << properties.deviceName << std::endl;
    }

    // 查询当前物理设备支持的队列属性
    void Context::queryQueueFamilyIndices() {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, nullptr);

        std::vector <VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, queueFamilies.data());

        for (int i = 0; i < queueFamilyCount; i++) {
            auto queueFamily = queueFamilies[i];
            // 支持图像操作的queueFamilyIndex
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queueFamilyIndices_.graphicsQueue = i;
            }
            // 查询是否支持显示
            VkBool32 presentSupport;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice_,
                                                 i,
                                                 surface_,
                                                 &presentSupport);
            if (presentSupport) {
                queueFamilyIndices_.presentQueue = i;
            }
            // 找到支持图像操作和显示的 deviceQueueFamily
            if (queueFamilyIndices_) {
                break;
            }
        }
    }

    /*
     * 逻辑设备和GPU交互，不能直接使用 物理设备
     * Queue  《===device_======》physicalDevice_ 传输 commandBuffer作为桥梁
     */
    void Context::createDevice() {
        std::array extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        // LogicDevice 可以设置拓展和层 extensions
        VkDeviceCreateInfo createInfo{};
        std::vector <VkDeviceQueueCreateInfo> queueCreateInfos;
        float queuePriority = 1.0f;

        // only need create one queue that supports both graphics and present
        if (queueFamilyIndices_.presentQueue.value() == queueFamilyIndices_.graphicsQueue.value()) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.queueFamilyIndex = queueFamilyIndices_.graphicsQueue.value();
            queueCreateInfos.emplace_back(queueCreateInfo);
            std::cout << "Using ONE same queue for both graphics and present " << std::endl;
        } else {
            // graphicsQueueCreateInfo
            VkDeviceQueueCreateInfo graphicsQueueCreateInfo{};
            graphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            graphicsQueueCreateInfo.pQueuePriorities = &queuePriority;
            graphicsQueueCreateInfo.queueCount = 1;
            graphicsQueueCreateInfo.queueFamilyIndex = queueFamilyIndices_.graphicsQueue.value();
            // presentQueueCreateInfo
            VkDeviceQueueCreateInfo presentQueueCreateInfo{};
            presentQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            presentQueueCreateInfo.pQueuePriorities = &queuePriority;
            presentQueueCreateInfo.queueCount = 1;
            presentQueueCreateInfo.queueFamilyIndex = queueFamilyIndices_.presentQueue.value();
            queueCreateInfos.emplace_back(graphicsQueueCreateInfo);
            queueCreateInfos.emplace_back(presentQueueCreateInfo);
            std::cout << "Using TWO queues for graphics and present " << std::endl;
        }

        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        if (vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create Vulkan device_.");
        }
        std::cout << "Vulkan device_ created" << std::endl;
    }

    void Context::QuitSwapChain() {
        swapchain_.reset();
    }

    void Context::getQueues() {
        vkGetDeviceQueue(device_, queueFamilyIndices_.graphicsQueue.value(), 0, &graphicsQueue_);
        vkGetDeviceQueue(device_, queueFamilyIndices_.presentQueue.value(), 0, &presentQueue_);
    }

    // 初始化 Swapchain
    void Context::InitSwapChain(int width, int height) {
        swapchain_ = std::make_shared<SwapChain>(width, height);
        swapchain_->getImages();
        swapchain_->CreateImageViews();
    }

    void Context::InitRenderProcess() {
        render_process_ = std::make_shared<RenderProcess>(device_, *swapchain_);
    }

    void Context::InitCommandManager() {
        commandManager_ = std::make_shared<CommandManager>(queueFamilyIndices_.graphicsQueue.value(), device_);
    }

    void Context::QuitCommandManager() {
        commandManager_.reset();
    }
}
