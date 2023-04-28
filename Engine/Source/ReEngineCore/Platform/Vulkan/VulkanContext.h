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
#include "vulkan/vulkan.h"
#include "GLFW/glfw3.h"
#include "glm/gtx/transform.hpp"
#include "Mesh/VulkanMesh.h"
#include "VulkanBuffers/VulkanBuffer.h"
#include "VulkanBuffers/VulkanDynamicBufferRing.h"
#include "VulkanBuffers/VulkanFrameBuffer.h"
#include "VulkanBuffers/VulkanIndexBuffer.h"
#include "VulkanBuffers/VulkanVertexBuffer.h"
#include "VulkanShader/VulkanShader.h"
#include "VulkanUI/VulkanImGui.h"

namespace ReEngine
{
    class VulkanContext : public GraphicsContext
    {
    public:
        
        VulkanContext(GLFWwindow* windowHandle,const WindowProperty* WinProperty);
        virtual ~VulkanContext()override{}

        virtual void Init() override; 
        virtual void Close() override;
        virtual void Acquire();
        virtual void SwapBuffers(Timestep ts) override;
        virtual void RecreateSwapChain();
        virtual VkCommandBuffer& GetCommandList();
        virtual int32 GetCurrtIndex();
        virtual void BeginUI();
        virtual void EndUI();
        virtual void DrawUI();
        [[nodiscard]]Ref<VulkanInstance> GetVulkanInstance(){ return Instance;}
        [[nodiscard]]GLFWwindow* GetGLFWwindow(){return m_WindowHandle;}
        
    public:
        Ref<VulkanInstance> Instance;
        Ref<VulkanCommandPool> CommandPool;
        
        GLFWwindow* m_WindowHandle;
        const WindowProperty* WinProperty;

        VulkanImGui* m_GUI;
        
        void CreateCommandBuffers();
        
        void CreateGUI();
        void DestroyGUI();

    };
}

