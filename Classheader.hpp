#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include <vector>
#include <set>
#include "Alloctor.hpp"

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

namespace vulkan {

    class VulkanApp {
    public:
        VulkanApp() : instance(VK_NULL_HANDLE), logicalDevice(VK_NULL_HANDLE), window(nullptr), swapchain(VK_NULL_HANDLE) {
            run();
        }
        GLFWwindow* window;
        void run() {
            CreateAllocator();
            createInstance();
            wininit();
            enumeratePhysicalDevices();
            createLogicalDevice();
            swapChainSupport = querySwapChainSupport(physicalDevices[0], surface);
            printSwapChainSupportDetails(swapChainSupport);
            createSwapChain(); // Encapsulated swapchain creation
            mainLoop();
            cleanup();
         
            
        }

        

        
        
    private:
        VkApplicationInfo appInfo{};
        VkInstanceCreateInfo createInfo{};
        VkInstance instance;
        VkAllocationCallbacks Alloctor;
        VkDevice logicalDevice;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        std::vector<VkPhysicalDevice> physicalDevices;
        VkSurfaceKHR surface;
        VkRenderPass renderPasss;
        std::vector<VkImageView> swapChainImageViews;
        std::vector<VkImage> swapChainImages;
        
        void cleanup() {
            if (surface)
            {
                vkDestroySurfaceKHR(instance, surface, &Alloctor);

            }
            if (swapchain != VK_NULL_HANDLE) {
                cleanupSwapChain();
            }
            if (logicalDevice != VK_NULL_HANDLE) {
                vkDestroyDevice(logicalDevice, &Alloctor);
            }
           

            if (instance != VK_NULL_HANDLE) {
                vkDestroyInstance(instance, &Alloctor);
            }
            if (window) {
                glfwDestroyWindow(window);
            }
            glfwTerminate();
        }
        VkSwapchainKHR swapchain; // Add a member for the swapchain
        SwapChainSupportDetails swapChainSupport; // Store swapchain support details
        int graphicsFamilyIndex = -1;
        int presentFamilyIndex = -1;

        void printSwapChainSupportDetails(const SwapChainSupportDetails& details) {
            // Print Surface Capabilities
            std::cout << "Surface Capabilities:" << std::endl;
            std::cout << " - Min Image Count: " << details.capabilities.minImageCount << std::endl;
            std::cout << " - Max Image Count: " << details.capabilities.maxImageCount << std::endl;
            std::cout << " - Current Extent (width x height): " << details.capabilities.currentExtent.width << " x " << details.capabilities.currentExtent.height << std::endl;
            std::cout << " - Min Image Extent (width x height): " << details.capabilities.minImageExtent.width << " x " << details.capabilities.minImageExtent.height << std::endl;
            std::cout << " - Max Image Extent (width x height): " << details.capabilities.maxImageExtent.width << " x " << details.capabilities.maxImageExtent.height << std::endl;
            std::cout << " - Max Image Array Layers: " << details.capabilities.maxImageArrayLayers << std::endl;

            // Print Supported Surface Formats
            std::cout << "Supported Surface Formats:" << std::endl;
            for (const auto& format : details.formats) {
                std::cout << " - Format: " << format.format << ", Color Space: " << format.colorSpace << std::endl;
            }

            // Print Available Presentation Modes
            std::cout << "Supported Presentation Modes:" << std::endl;
            for (const auto& presentMode : details.presentModes) {
                std::string mode;
                switch (presentMode) {
                case VK_PRESENT_MODE_IMMEDIATE_KHR: mode = "VK_PRESENT_MODE_IMMEDIATE_KHR"; break;
                case VK_PRESENT_MODE_MAILBOX_KHR: mode = "VK_PRESENT_MODE_MAILBOX_KHR"; break;
                case VK_PRESENT_MODE_FIFO_KHR: mode = "VK_PRESENT_MODE_FIFO_KHR"; break;
                case VK_PRESENT_MODE_FIFO_RELAXED_KHR: mode = "VK_PRESENT_MODE_FIFO_RELAXED_KHR"; break;
                default: mode = "Unknown Mode"; break;
                }
                std::cout << " - " << mode << std::endl;
            }
        }

        void createLogicalDevice() {
            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[0], &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[0], &queueFamilyCount, queueFamilies.data());

            
            for (int i = 0; i < queueFamilies.size(); i++) {
                if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    graphicsFamilyIndex = i;
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[0], i, surface, &presentSupport);
                if (presentSupport == VK_TRUE) {
                    presentFamilyIndex = i;
                }

                if (graphicsFamilyIndex != -1 && presentFamilyIndex != -1) {
                    break;
                }
            }
            if (graphicsFamilyIndex == -1 || presentFamilyIndex == -1) {
                throw std::runtime_error("Failed to find suitable queue families!");
            }
            float queuePriority = 1.0f;
            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set<int> uniqueQueueFamilies = { graphicsFamilyIndex, presentFamilyIndex };

            for (int queueFamily : uniqueQueueFamilies) {
                VkDeviceQueueCreateInfo queueCreateInfo{};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(queueCreateInfo);
            }

            VkPhysicalDeviceFeatures deviceFeatures{}; // Initialize device features if needed
            std::vector<const char*> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
            };

            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.pQueueCreateInfos = queueCreateInfos.data();
            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pEnabledFeatures = &deviceFeatures;
            createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
            createInfo.ppEnabledExtensionNames = deviceExtensions.data();

            if (vkCreateDevice(physicalDevices[0], &createInfo, &Alloctor, &logicalDevice) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create logical device!");
            }
            else {
                std::cout << "Created logical device." << std::endl;
                printPhysicalDeviceProperties(physicalDevices[0], 0);
            }

            vkGetDeviceQueue(logicalDevice, graphicsFamilyIndex, 0, &graphicsQueue);
            vkGetDeviceQueue(logicalDevice, presentFamilyIndex, 0, &presentQueue);
        }

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
            SwapChainSupportDetails details;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
            if (formatCount != 0) {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
            }

            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
            if (presentModeCount != 0) {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
            }

            return details;
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
            glfwSetWindowUserPointer(window, this);
            glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        }
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
            auto app = reinterpret_cast<VulkanApp*>(glfwGetWindowUserPointer(window));
            app->recreateSwapChain();
        }

        void recreateSwapChain() {
            int width = 0, height = 0;
            glfwGetFramebufferSize(window, &width, &height);
            if (width == 0 || height == 0) {
                return; // Do not recreate if the window is minimized
            }

            vkDeviceWaitIdle(logicalDevice); // Wait for the device to be idle

            // Cleanup previous swap chain
            cleanupSwapChain();

            // Recreate swap chain
            swapChainSupport = querySwapChainSupport(physicalDevices[0], surface);
            createSwapChain();

            // Create image views and framebuffers for the new swap chain
            createSwapChainImageViews(logicalDevice, swapChainImages);
            createFramebuffers();
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

        void createSwapChain() {
            // Select the best surface format and present mode
            
            VkSurfaceFormatKHR surfaceFormat = selectSurfaceFormat(swapChainSupport.formats);
            VkPresentModeKHR presentMode = selectPresentMode(swapChainSupport.presentModes);

            VkExtent2D extent = swapChainSupport.capabilities.currentExtent;
            uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
            if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
                imageCount = swapChainSupport.capabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = surface;
            createInfo.minImageCount = imageCount;
            createInfo.imageFormat = surfaceFormat.format;
            createInfo.imageColorSpace = surfaceFormat.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            
            // Specify queue families
            uint32_t queueFamilyIndices[] = { graphicsFamilyIndex, presentFamilyIndex };
            if (graphicsFamilyIndex != presentFamilyIndex) {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            }
            else {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                createInfo.queueFamilyIndexCount = 0; // Optional
                createInfo.pQueueFamilyIndices = nullptr; // Optional
            }

            createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE;
            createInfo.oldSwapchain = VK_NULL_HANDLE;

            if (vkCreateSwapchainKHR(logicalDevice, &createInfo, &Alloctor, &swapchain) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create swap chain!");
            }
            std::cout << "Created swap chain." << std::endl;
            
            vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, nullptr);
            swapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(logicalDevice, swapchain, &imageCount, swapChainImages.data());
        }
        void cleanupSwapChain() {
            vkDestroySwapchainKHR(logicalDevice , swapchain , &Alloctor);
            for (auto imageView : swapChainImageViews) {
                vkDestroyImageView(logicalDevice, imageView, &Alloctor);
            }
            for (auto framebuffer : framebuffers) {
                vkDestroyFramebuffer(logicalDevice, framebuffer, &Alloctor);
            }
            
        }
        VkSurfaceFormatKHR selectSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
            for (const auto& format : availableFormats) {
                if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return format; // Found a suitable format
                }
            }
            // If we didn't find a suitable format, just return the first available one
            return availableFormats[0];
        }

        VkPresentModeKHR selectPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
            for (const auto& mode : availablePresentModes) {
                if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    return mode; // Prefer mailbox for lower latency
                }
            }
            return VK_PRESENT_MODE_FIFO_KHR; // Fallback to FIFO
        }
        VkRenderPass createRenderPass(VkDevice device) {
            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB; // Match swap chain format
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear the attachment
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Store the result
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // No initial layout
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Final layout for presentation

            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = 0; // Attachment index
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;

            VkRenderPassCreateInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = 1;
            renderPassInfo.pAttachments = &colorAttachment;
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;
            

            if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPasss) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create render pass!");
            }
            std::cout << "Created swap chain." << std::endl;

            return renderPasss;
        }
        

        void createSwapChainImageViews(VkDevice device, const std::vector<VkImage>& swapChainImages) {
            swapChainImageViews.resize(swapChainImages.size());

            for (size_t i = 0; i < swapChainImages.size(); i++) {
                VkImageViewCreateInfo viewInfo{};
                viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewInfo.image = swapChainImages[i];
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewInfo.format = VK_FORMAT_B8G8R8A8_SRGB; // Match swap chain format
                viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
                viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
                viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
                viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewInfo.subresourceRange.baseMipLevel = 0;
                viewInfo.subresourceRange.levelCount = 1;
                viewInfo.subresourceRange.baseArrayLayer = 0;
                viewInfo.subresourceRange.layerCount = 1;

                if (vkCreateImageView(device, &viewInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create image views!");
                }
                else
                {
                    std::cout << "createSwapChainImageViews is working  \n";
                }
            }
        }
        std::vector<VkFramebuffer> framebuffers;
        void createFramebuffers() {
            // Resize the framebuffer list to match the number of swap chain image views
            framebuffers.resize(swapChainImageViews.size());

            // Loop through all image views and create framebuffers
            for (size_t i = 0; i < swapChainImageViews.size(); i++) {
                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPasss;;  // Use the render pass created earlier
                framebufferInfo.attachmentCount = 1;      // Number of attachments (we're only using the color attachment)
                framebufferInfo.pAttachments = &swapChainImageViews[i];  // Image view for this framebuffer
                framebufferInfo.width = swapChainSupport.capabilities.currentExtent.width;   // Swap chain image width
                framebufferInfo.height = swapChainSupport.capabilities.currentExtent.height; // Swap chain image height
                framebufferInfo.layers = 1;  // Number of layers in the image (1 for 2D images)

                // Create the framebuffer for this image view
                if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, &Alloctor, &framebuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create framebuffer!");
                }
            }

            std::cout << "Framebuffers created successfully." << std::endl;
        }
    };
    


    void runn() {
        auto app = std::make_unique<VulkanApp>();
        // Application runs and resources are cleaned up automatically
    }

} // namespace vulkan
