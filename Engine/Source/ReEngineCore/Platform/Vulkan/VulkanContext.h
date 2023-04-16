#pragma once
#include "Core/PCH.h"
#include "Renderer/RHI/GraphicsContext.h"
#include "Renderer/RHI/PixelFormat.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include "VulkanCommandPool.h"
#include "VulkanInstance.h"
#include "VulkanPipelineInfo.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanTexture.h"
#include "Camera/EditorCamera.h"
#include "vulkan/Include/vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "glm/gtx/transform.hpp"
#include "Mesh/VulkanMesh.h"
#include "VulkanBuffers/VulkanBuffer.h"
#include "VulkanBuffers/VulkanDynamicBufferRing.h"
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

    struct ParamBlock
    {
        glm::vec3 lightDir;
        float curvature;

        glm::vec3 lightColor;
        float exposure;

        glm::vec2 curvatureScaleBias;
        float blurredLevel;
        float padding;
    };
    
    class VulkanContext : public GraphicsContext
    {
    public:
        UniformBufferObject ubo;
        ParamBlock Param;
        
        VulkanContext(GLFWwindow* windowHandle,const WindowProperty* WinProperty);
        virtual ~VulkanContext()override{}

        virtual void Init() override; 
        virtual void Close() override;
        virtual void SwapBuffers(Timestep ts) override;
        virtual void RecreateSwapChain();
        [[nodiscard]]Ref<VulkanInstance> GetVulkanInstance(){ return Instance;}
        [[nodiscard]]GLFWwindow* GetGLFWwindow(){return m_WindowHandle;}
        
    private:
        Ref<VulkanInstance> Instance;
        Ref<VulkanCommandPool> CommandPool;
        Ref<VulkanFrameBuffer> FrameBuffer;
        Ref<VulkanPipeline> GraphicsPipeline;

        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorPool descriptorPool;
        VkDescriptorSet descriptorSet;
        VkPipelineLayout pipelineLayout;
        
        // Ref<VulkanBuffer> UniformBuffer;
        // Ref<VulkanBuffer> ParamBuffer;
        Ref<VulkanDynamicBufferRing> RingBuffer;
        
        Ref<VulkanModel> Model;
        Ref<VulkanTexture> TexDiffuse;
        Ref<VulkanTexture> TexNomal;
        Ref<VulkanTexture> TexPreIntegareted;
        Ref<VulkanTexture> TexCurve;
        Ref<EditorCamera> Camera;
        
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
        
        void CommitCmd();
        void OnEvent(std::shared_ptr<Event> e);
        void CreateGUI();
        void DestroyGUI();
        bool UpdateUI(float time,float delta);
    };
}

