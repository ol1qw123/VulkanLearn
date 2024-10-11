#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include <vector>
#include "Alloctor.hpp"

namespace vulkan {

    class VulkanApp {
    public:
        VulkanApp() : instance(VK_NULL_HANDLE), logicalDevice(VK_NULL_HANDLE), window(nullptr) {
            run();
        }

        void run() {
            CreateAllocator();
            createInstance();
            wininit();
            enumeratePhysicalDevices();
            createLogicalDevice();
            mainLoop();
            cleanup();
        }

        ~VulkanApp() {
            cleanup();
        }

        void cleanup() {
            if (logicalDevice != VK_NULL_HANDLE) {
                vkDestroyDevice(logicalDevice, &Alloctor);
            }
            if (surface != VK_NULL_HANDLE) {
                vkDestroySurfaceKHR(instance, surface, &Alloctor);
            }
            if (instance != VK_NULL_HANDLE) {
                vkDestroyInstance(instance, &Alloctor);
                instance = VK_NULL_HANDLE;
            }
            if (window) {
                glfwDestroyWindow(window);
            }
            glfwTerminate();
        }

    private:
        VkApplicationInfo appInfo{};
        VkInstanceCreateInfo createInfo{};
        VkInstance instance;
        VkAllocationCallbacks Alloctor;
        VkDevice logicalDevice;
        VkQueue graphicsQueue;
        std::vector<VkPhysicalDevice> physicalDevices;
        VkSurfaceKHR surface;
        GLFWwindow* window;

        void createLogicalDevice() {
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[0], &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[0], &queueFamilyCount, queueFamilies.data());

            int graphicsFamilyIndex = -1;
            for (int i = 0; i < queueFamilies.size(); i++) {
                if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    graphicsFamilyIndex = i;
                    break;
                }
            }

            if (graphicsFamilyIndex == -1) {
                throw std::runtime_error("Failed to find a suitable queue family!");
            }

            float queuePriority = 1.0f;
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = graphicsFamilyIndex;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            VkPhysicalDeviceFeatures deviceFeatures{};

            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.pQueueCreateInfos = &queueCreateInfo;
            createInfo.queueCreateInfoCount = 1;
            createInfo.pEnabledFeatures = &deviceFeatures;

            if (vkCreateDevice(physicalDevices[0], &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create logical device!");
            }
            else {
                std::cout << "Created logical device." << std::endl;
                printPhysicalDeviceProperties(physicalDevices[0], 0);
            }

            vkGetDeviceQueue(logicalDevice, graphicsFamilyIndex, 0, &graphicsQueue);
        }

        void CreateAllocator() {
            Alloctor = {};  // Initialize to zero
            Alloctor.pUserData = nullptr;
            Alloctor.pfnAllocation = Allocator::CustomAllocation;
            Alloctor.pfnReallocation = Allocator::CustomReallocation;
            Alloctor.pfnFree = Allocator::CustomFree;
            // Optional callbacks
            Alloctor.pfnInternalAllocation = nullptr;
            Alloctor.pfnInternalFree = nullptr;
        }

        void createInstance() {
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Hello Vulkan";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_3;

            const char* extensions[] = {
                VK_KHR_SURFACE_EXTENSION_NAME,
                "VK_KHR_win32_surface"  // Add this line for Windows
            };

            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;
            createInfo.enabledExtensionCount = 2; // Number of extensions
            createInfo.ppEnabledExtensionNames = extensions;

            if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create Vulkan instance");
            }
        }

        void wininit() {
            if (!glfwInit()) {
                throw std::runtime_error("Failed to initialize GLFW");
            }

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            window = glfwCreateWindow(800, 600, "Vulkan Window", nullptr, nullptr);
            if (!window) {
                throw std::runtime_error("Failed to create GLFW window");
            }

            // Create the surface
            if (glfwCreateWindowSurface(instance, window, &Alloctor, &surface) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create window surface");
            }
        }

        void mainLoop() {
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();  // Handle events
            }
        }

        void enumeratePhysicalDevices() {
            uint32_t deviceCount = 0;
            if (vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr) != VK_SUCCESS || deviceCount == 0) {
                throw std::runtime_error("Failed to find GPUs with Vulkan support!");
            }

            physicalDevices.resize(deviceCount);
            if (vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data()) != VK_SUCCESS) {
                throw std::runtime_error("Failed to enumerate physical devices!");
            }

            std::cout << "Number of physical devices: " << deviceCount << std::endl;
            for (size_t i = 0; i < physicalDevices.size(); ++i) {
                printPhysicalDeviceProperties(physicalDevices[i], i + 1);
            }
        }

        void printPhysicalDeviceProperties(VkPhysicalDevice device, int deviceIndex) {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);

            std::cout << "Physical Device " << deviceIndex << ": " << properties.deviceName << std::endl;
            std::cout << "API Version: "
                << VK_VERSION_MAJOR(properties.apiVersion) << "."
                << VK_VERSION_MINOR(properties.apiVersion) << "."
                << VK_VERSION_PATCH(properties.apiVersion) << std::endl;
            std::cout << "Driver Version: " << properties.driverVersion << std::endl;
            std::cout << "Vendor ID: " << properties.vendorID << std::endl;
            std::cout << "Device ID: " << properties.deviceID << std::endl;

            std::cout << "Device Type: ";
            switch (properties.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                std::cout << "Integrated GPU";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                std::cout << "Discrete GPU";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                std::cout << "Virtual GPU";
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                std::cout << "CPU";
                break;
            default:
                std::cout << "Unknown";
                break;
            }
            std::cout << std::endl << std::endl;
        }

       
    };

    void runn(){
        auto app = std::make_unique<VulkanApp>();
        // Application runs and resources are cleaned up automatically
    }

} // namespace vulkan
