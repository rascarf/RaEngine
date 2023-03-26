#pragma once
#include "Core/PCH.h"
#include "Renderer/RHI/GraphicsContext.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/Include/vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "Core/Window/Window.h"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

namespace ReEngine
{
    struct QueueFamilyIndices
    {
        int GraphicsFamily = -1;
        int PresentFamily = -1;
        bool isComplete()
        {
            return GraphicsFamily >= 0 && PresentFamily >= 0;
        }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    
    
    class VulkanContext : public GraphicsContext
    {
    public:
        VulkanContext(GLFWwindow* windowHandle,const WindowProperty* WinProperty);

        virtual void Init() override; // Load OpenGL Context
        virtual void Close() override;
        virtual void SwapBuffers() override;

        void RecreateSwapChain();
    private:
        const WindowProperty* WinProperty;
        
        VkInstance Instance;
        VkPhysicalDevice physicalDevice; //物理设备
        VkDevice Device; //逻辑设备
        VkDebugReportCallbackEXT callback;

        //支持计算和呈现Command簇的显卡可能不同
        VkQueue graphicsQueue; 
        VkQueue PresentQueue;
        
        VkSurfaceKHR surface;

        VkSwapchainKHR swapChain;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> SwapChainImageViews;

        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorPool descriptorPool;
        VkDescriptorSet descriptorSet;
        
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;

        VkRenderPass renderPass;
        
        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;

        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;

        VkBuffer VertexBuffer;
        VkDeviceMemory vertexBufferMemory;

        VkBuffer IndexBuffer;
        VkDeviceMemory IndexBufferMemory;

        VkBuffer uniformBuffer;
        VkDeviceMemory uniformBufferMemory;
        
        GLFWwindow* m_WindowHandle;
        
        void InitVulkan();
        void CreateSurface();
        std::vector<const char*> GetRequiredExtensions();
        
        bool CheckValidationLayerSupport();
        void SetupDebugCallBack();
        
        void createLogicalDevice();
        void PickPhysicalDevices();
        bool isDeviceSuitable(VkPhysicalDevice device);
        bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
        QueueFamilyIndices FindQueueFamily(VkPhysicalDevice device);
        
        void CreateSwapChain();
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        VkShaderModule CreateShaderModule(const std::vector<char>& code);
        void CreateImageViews();
        void CreateRenderPass();
        void CreateGraphicsPipeline();
        void CreateFrameBuffer();
        void CreateCommandPool();
        void CreateCommandBuffers();
        void CreateSemaphores();
        void ClearSwapChain();
        
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        void CreateVertexBuffer();
        void CreateIndexBuffer();
        void createUniformBuffer();
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void CopyBuffer(VkBuffer SrcBuffer,VkBuffer DstBuffer,VkDeviceSize size);

        void CreateDescriptorSetLayout();
        void UpdateUniformBuffer();
        void CreateDescriptorPool();
        void CreateDescriptorSet();
    };

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags,VkDebugReportObjectTypeEXT objType,uint64_t obj,size_t location,int32_t code,const char* layerPrefix,const char* msg,void* userData);
}

