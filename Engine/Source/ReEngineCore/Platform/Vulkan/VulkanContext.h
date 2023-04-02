#pragma once
#include "Core/PCH.h"
#include "Renderer/RHI/GraphicsContext.h"
#include "Renderer/RHI/PixelFormat.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include "VulkanCommandPool.h"
#include "VulkanInstance.h"
#include "vulkan/Include/vulkan/vulkan.h"
#include "VulkanBuffer.h"
#include "GLFW/glfw3.h"

namespace ReEngine
{
    class VulkanContext : public GraphicsContext
    {
    public:
        VulkanContext(GLFWwindow* windowHandle,const WindowProperty* WinProperty);

        virtual void Init() override; 
        virtual void Close() override;
        virtual void SwapBuffers() override;
        [[nodiscard]]Ref<VulkanInstance> GetVulkanInstance(){ return Instance;}
        
    private:
        Ref<VulkanInstance> Instance;
        Ref<VulkanCommandPool> CommandPool;

        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorPool descriptorPool;
        VkDescriptorSet descriptorSet;
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;

        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkRenderPass renderPass;
        
        VulkanBuffer* UniformBuffer;
        VulkanBuffer* VertexBuffer;
        VulkanBuffer* IndexBuffer;
        
        GLFWwindow* m_WindowHandle;
        const WindowProperty* WinProperty;
        
        void CreateFrameBuffer();
        void CreateRenderPass();
        void CreateGraphicsPipeline();
        VkShaderModule CreateShaderModule(const std::vector<unsigned char>& code);
        void CreateDepthStencil();
        
        void CreateMeshBuffer();
        void createUniformBuffer();
        void CreateCommandBuffers();

        void CreateDescriptorSetLayout();
        void CreateDescriptorPool();
        void CreateDescriptorSet();
        void UpdateUniformBuffer();
        
    };
}

