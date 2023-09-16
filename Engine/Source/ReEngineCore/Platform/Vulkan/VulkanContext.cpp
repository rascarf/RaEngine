#include "VulkanContext.h"

#include <array>
#include <memory>

#include "Shader_vert.h"

#include "spirv_cross.hpp"
#include "imgui/imgui.h"
#include "VulkanCommandPool.h"
#include "VulkanInstance.h"
#include "Core/Timestep.h"
#include "Resource/AssetManager/AssetManager.h"
#include "VulkanShader/VulkanShader.h"


namespace ReEngine
{
    VulkanContext::VulkanContext(GLFWwindow* windowHandle,const WindowProperty* InWindowProperty):m_WindowHandle(windowHandle),WinProperty(InWindowProperty)
    {
    	Instance = CreateRef<VulkanInstance>(windowHandle,InWindowProperty);
    	CommandPool = CreateRef<VulkanCommandPool>();
    }

    void VulkanContext::Init()
    {
        Instance->Init();
    	CommandPool->Init(this);
    	CreateCommandBuffers();
    	CreateGUI();
    }

    void VulkanContext::Close()
    {
        const auto Device = Instance->GetDevice()->GetInstanceHandle();
        vkDeviceWaitIdle(Device);

    	m_GUI->Destroy();

    	CommandPool->ShutDown();
        Instance->Shutdown();
    }

    void VulkanContext::Acquire()
    {
    	const int32 bufferIndex = CommandPool->AcquireBackbufferIndex();
    	if (bufferIndex < 0 ) return;

    	//分配当前帧的CmdList
    	VkCommandBufferAllocateInfo allocInfo = {};
    	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    	allocInfo.commandPool = CommandPool->m_CommandPool;
    	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    	allocInfo.commandBufferCount = 1;
    
    	if (vkAllocateCommandBuffers(Instance->GetDevice()->GetInstanceHandle(), &allocInfo, &CommandPool->m_CommandBuffers[GetCurrtIndex()]) != VK_SUCCESS)
    	{
    		RE_CORE_ERROR("Failed to allocate command buffers!");
    	}
    }

    void VulkanContext::SwapBuffers(Timestep ts)
    {
    	if (vkEndCommandBuffer(GetCommandList()) != VK_SUCCESS)
    	{
    		throw std::runtime_error("failed to record command buffer!");
    	}
    	
		CommandPool->Present(GetCurrtIndex() < 0 ? 0 : GetCurrtIndex());
    }

    void VulkanContext::RecreateSwapChain()
    {
    	CommandPool->ShutDown();
    	
    	Instance->RecreateSwapChain();
    	CommandPool->Init(this);
    	
    	CreateCommandBuffers();
    }

    VkCommandBuffer& VulkanContext::GetCommandList()
    {
    	return CommandPool->m_CommandBuffers[GetCurrtIndex()];
    }

    int32 VulkanContext::GetCurrtIndex()
    {
    	return CommandPool->m_FrameIndex;
    }

    void VulkanContext::BeginUI()
    {
    	m_GUI->StartFrame();
    }

    void VulkanContext::EndUI()
    {
    	m_GUI->EndFrame();

    	//获取渲染数据
    	m_GUI->Update();
    }

    void VulkanContext::DrawUI()
    {
    	VkRenderPassBeginInfo UIrenderPassInfo = {};
    	UIrenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    	UIrenderPassInfo.renderPass = m_GUI->m_UIRenderPass;
    	UIrenderPassInfo.framebuffer = m_GUI->m_UIFrameBuffers[GetCurrtIndex()];
    	UIrenderPassInfo.renderArea.offset = { 0, 0 };
    	UIrenderPassInfo.renderArea.extent = VkExtent2D(Instance->GetSwapChain()->GetWidth(),Instance->GetSwapChain()->GetHeight());
    
    	vkCmdBeginRenderPass(GetCommandList(), &UIrenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    	m_GUI->BindDrawCmd(GetCommandList(),m_GUI->m_UIRenderPass);
    
    	vkCmdEndRenderPass(GetCommandList());
    }

    void VulkanContext::CreateCommandBuffers()
    {
        CommandPool->m_CommandBuffers.resize(CommandPool->m_SwapChain->GetBackBufferCount());

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = CommandPool->m_CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)CommandPool->m_CommandBuffers.size();

        if (vkAllocateCommandBuffers(Instance->GetDevice()->GetInstanceHandle(), &allocInfo, CommandPool->m_CommandBuffers.data()) != VK_SUCCESS)
        {
            RE_CORE_ERROR("Failed to allocate command buffers!");
        }
    }
	
    void VulkanContext::CreateGUI()
    {
    	m_GUI = new VulkanImGui();
    	m_GUI->Init("assets/fonts/Ubuntu-Regular.ttf",this);
    }

    void VulkanContext::DestroyGUI()
    {
    	m_GUI->Destroy();
    	delete m_GUI;
    }
}

