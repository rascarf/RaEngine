#pragma once
#include "Core/PCH.h"
#include "Renderer/RHI/GraphicsContext.h"
#include "Renderer/RHI/PixelFormat.h"

#include "VulkanCommandPool.h"
#include "VulkanInstance.h"
#include "GLFW/glfw3.h"
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

