#pragma once
#include <vulkan/vulkan.hpp>
#include <stdexcept>
#include <iostream>
#include <vector>
#include "Alloctor.hpp"


namespace vulkan {

    // Encapsulate Vulkan instance within a class to manage its lifecycle
    class VulkanApp {
    public:
        VulkanApp() : instance(VK_NULL_HANDLE) {
            setupAppInfo();
            setupInstanceCreateInfo();
            createInstance();
            CreateAlloctor();
            enumeratePhysicalDevices();
        }

        ~VulkanApp() {
            destroyInstance();
        }

    private:
        VkApplicationInfo appInfo;
        VkInstanceCreateInfo createInfo;
        VkInstance instance;
        VkAllocationCallbacks Alloctor;
        void setupAppInfo() {
            appInfo = {};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = "Test 1";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_3;
        }
        void CreateAlloctor() {
            Alloctor.pUserData = NULL; // No user data
            Alloctor.pfnAllocation = Alloctor::CustomAllocation;
            Alloctor.pfnReallocation = Alloctor::CustomReallocation;
            Alloctor.pfnFree = Alloctor::CustomFree;
            Alloctor.pfnInternalAllocation = NULL; // Optional
            Alloctor.pfnInternalFree = NULL; // Optional
        }
        void setupInstanceCreateInfo() {
            createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            // No extensions needed for this simple example.
            std::vector<const char*> extensions = {
                // VK_KHR_SURFACE_EXTENSION_NAME // Omit for now
            };

            createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
            createInfo.ppEnabledExtensionNames = extensions.empty() ? nullptr : extensions.data();

            // No validation layers for simplicity. Add if needed.
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;

            createInfo.flags = 0;
        }

        void createInstance() {
            VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Failed to create Vulkan instance!");
            }
        }

        void enumeratePhysicalDevices() {
            uint32_t deviceCount = 0;
            VkResult result = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
            if (result != VK_SUCCESS || deviceCount == 0) {
                throw std::runtime_error("Failed to find GPUs with Vulkan support!");
            }

            std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
            result = vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());
            if (result != VK_SUCCESS) {
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

        void destroyInstance() {
            if (instance != VK_NULL_HANDLE) {
                vkDestroyInstance(instance, nullptr);
                instance = VK_NULL_HANDLE;
            }
        }
    };

    void run() {
        VulkanApp app;
        // Application runs and resources are cleaned up automatically
    }

} // namespace vulkan