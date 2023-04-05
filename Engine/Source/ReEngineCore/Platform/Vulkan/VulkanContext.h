#pragma once
#include "Core/PCH.h"
#include "Renderer/RHI/GraphicsContext.h"
#include "Renderer/RHI/PixelFormat.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include "VulkanCommandPool.h"
#include "VulkanInstance.h"
#include "vulkan/Include/vulkan/vulkan.h"


#include "GLFW/glfw3.h"
#include "glm/gtx/transform.hpp"
#include "VulkanBuffers/VulkanBuffer.h"
#include "VulkanBuffers/VulkanFrameBuffer.h"
#include "VulkanBuffers/VulkanIndexBuffer.h"
#include "VulkanBuffers/VulkanVertexBuffer.h"
#include "VulkanUI/VulkanImGui.h"

namespace ReEngine
{
    struct UniformBufferObject 
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
    
    class VulkanContext : public GraphicsContext
    {
    public:
        UniformBufferObject ubo = {};
        
        VulkanContext(GLFWwindow* windowHandle,const WindowProperty* WinProperty);

        virtual void Init() override; 
        virtual void Close() override;
        virtual void SwapBuffers(Timestep ts) override;
        virtual void RecreateSwapChain();
        [[nodiscard]]Ref<VulkanInstance> GetVulkanInstance(){ return Instance;}
        
    private:
        Ref<VulkanInstance> Instance;
        Ref<VulkanCommandPool> CommandPool;
        Ref<VulkanFrameBuffer> FrameBuffer;

        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorPool descriptorPool;
        VkDescriptorSet descriptorSet;
        VkPipelineLayout pipelineLayout;
        VkPipeline graphicsPipeline;

        Ref<VulkanBuffer> UniformBuffer;
        Ref<VulkanVertexBuffer> VertexBuffer;
        Ref<VulkanIndexBuffer> IndexBuffer;
        
        GLFWwindow* m_WindowHandle;
        const WindowProperty* WinProperty;

        VulkanImGui* m_GUI;
        
        void CreateGraphicsPipeline();
        VkShaderModule CreateShaderModule(const std::vector<unsigned char>& code);
        
        void CreateMeshBuffer();
        void createUniformBuffer();
        void CreateCommandBuffers();

        void CreateDescriptorSetLayout();
        void CreateDescriptorPool();
        void CreateDescriptorSet();
        void UpdateUniformBuffer(Timestep ts);

        void CreateGUI();
        void DestroyGUI();
        bool UpdateUI(float time,float delta);
    };
}

