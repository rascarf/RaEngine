﻿#include "VulkanContext.h"

#include <array>
#include <set>
#include <limits>
#include <fstream>
#include <memory>

#include "Shader_vert.h"
#include "Shader_frag.h"

#include "imgui/imgui.h"

#include "VulkanCommandPool.h"
#include "VulkanInstance.h"
#include "Core/Timestep.h"
#include "Resource/AssetManager/AssetManager.h"

namespace ReEngine
{
    VulkanContext::VulkanContext(GLFWwindow* windowHandle,const WindowProperty* InWindowProperty):m_WindowHandle(windowHandle),WinProperty(InWindowProperty)
    {
    	Instance = CreateRef<VulkanInstance>(windowHandle,InWindowProperty);
    	CommandPool = CreateRef<VulkanCommandPool>();
    	FrameBuffer = CreateRef<VulkanFrameBuffer>(InWindowProperty->Width,InWindowProperty->Height,InWindowProperty->Title.c_str());
    }

    void VulkanContext::Init()
    {
        Instance->Init();
    	CommandPool->Init(this);
    	FrameBuffer->Init(this);
    	CreateGUI();
    	
		CreateMeshBuffer();
        createUniformBuffer();
        CreateDescriptorSetLayout();
        CreateDescriptorPool();
        CreateDescriptorSet();
        CreateGraphicsPipeline();
    	CreateCommandBuffers();
    }

    void VulkanContext::Close()
    {
        const auto Device = Instance->GetDevice()->GetInstanceHandle();
        
        vkDeviceWaitIdle(Device);
    	
        vkDestroyDescriptorPool(Device, descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(Device, descriptorSetLayout, nullptr);
    	vkDestroyPipelineLayout(Device,pipelineLayout,nullptr);
    	vkDestroyPipeline(Device,graphicsPipeline,nullptr);

    	FrameBuffer->ShutDown();
    	CommandPool->ShutDown();
        Instance->Shutdown();
    }

    void VulkanContext::SwapBuffers(Timestep ts)
    {
    	bool hovered = UpdateUI(ts.GetMilliseconds(),ts.GetSeconds());
    	
        UpdateUniformBuffer(ts);
  		int32 bufferIndex = CommandPool->AcquireBackbufferIndex();
    	if(bufferIndex < 0)
    	{
    		return;
    	}
    	
		CommandPool->Present(bufferIndex);
    }

    void VulkanContext::RecreateSwapChain()
    {
    	const auto Device = Instance->GetDevice()->GetInstanceHandle();
    	
    	FrameBuffer->ShutDown();
    	CommandPool->ShutDown();
    	

    	vkDestroyPipeline(Device, graphicsPipeline, nullptr);
    	vkDestroyPipelineLayout(Device, pipelineLayout, nullptr);
    	
    	Instance->RecreateSwapChain();
    	FrameBuffer->Init(this);
    	CommandPool->Init(this);

    	CreateDescriptorSetLayout();
    	CreateDescriptorPool();
    	CreateDescriptorSet();
    	CreateGraphicsPipeline();
    	CreateCommandBuffers();
    }

    const std::vector<float> vertices = {
          1.0f,  1.0f, 0.0f , 1.0f, 0.0f, 0.0f,
         -1.0f,  1.0f, 0.0f , 0.0f, 1.0f, 0.0f,
          0.0f, -1.0f, 0.0f , 0.0f, 0.0f, 1.0f,
    };

    //根据点的个数，可以使用uint16_：65535
    const std::vector<uint16_t> indices = {
        0, 1, 2
    };
	
    
    void VulkanContext::CreateGraphicsPipeline()
    {
        VkShaderModule vertShaderModule = CreateShaderModule(SHADER_VERT);
		VkShaderModule fragShaderModule = CreateShaderModule(SHADER_FRAG);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    	auto VertexDeclare = Model->GetInputBinding();
    	auto InputDeclare = Model->GetInputAttributes();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &VertexDeclare;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(InputDeclare.size());
        vertexInputInfo.pVertexAttributeDescriptions = InputDeclare.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)WinProperty->Width;
		viewport.height = (float)WinProperty->Height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = VkExtent2D((float)WinProperty->Width,(float)WinProperty->Height);

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE; //现在没有剔除
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

		if (vkCreatePipelineLayout(Instance->GetDevice()->GetInstanceHandle(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}

    	VkPipelineDepthStencilStateCreateInfo depthStencilState;
    	ZeroVulkanStruct(depthStencilState, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
    	depthStencilState.depthTestEnable       = VK_TRUE;
    	depthStencilState.depthWriteEnable      = VK_TRUE;
    	depthStencilState.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
    	depthStencilState.depthBoundsTestEnable = VK_FALSE;
    	depthStencilState.back.failOp           = VK_STENCIL_OP_KEEP;
    	depthStencilState.back.passOp           = VK_STENCIL_OP_KEEP;
    	depthStencilState.back.compareOp        = VK_COMPARE_OP_ALWAYS;
    	depthStencilState.stencilTestEnable     = VK_FALSE;
    	depthStencilState.front                 = depthStencilState.back;
    	
		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
    	pipelineInfo.pDepthStencilState = &depthStencilState;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = FrameBuffer->m_RenderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(Instance->GetDevice()->GetInstanceHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		vkDestroyShaderModule(Instance->GetDevice()->GetInstanceHandle(), fragShaderModule, nullptr);
		vkDestroyShaderModule(Instance->GetDevice()->GetInstanceHandle(), vertShaderModule, nullptr);
    }
	
    VkShaderModule VulkanContext::CreateShaderModule(const std::vector<unsigned char>& code)
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();

        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(Instance->GetDevice()->GetInstanceHandle(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
        {
            RE_CORE_ERROR("failed to create shader module!");
        }

        return shaderModule;
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

        for (size_t i = 0; i < CommandPool->m_CommandBuffers.size(); i++)
        {
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

            vkBeginCommandBuffer(CommandPool->m_CommandBuffers[i], &beginInfo);

            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = FrameBuffer->m_RenderPass;
            renderPassInfo.framebuffer = FrameBuffer->m_FrameBuffers[i];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = VkExtent2D(Instance->GetSwapChain()->GetWidth(),Instance->GetSwapChain()->GetHeight());

        	VkClearValue clearValues[2];
        	clearValues[0].color        = {{0.2f, 0.2f, 0.2f, 1.0f}};
        	clearValues[1].depthStencil = { 1.0f, 0 };
        	
            renderPassInfo.clearValueCount = 2;
            renderPassInfo.pClearValues = clearValues;

            vkCmdBeginRenderPass(CommandPool->m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(CommandPool->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
            vkCmdBindDescriptorSets(CommandPool->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

        	for (int32 meshIndex = 0; meshIndex < Model->Meshes.size(); ++meshIndex)
        	{
        		Model->Meshes[meshIndex]->BindDraw(CommandPool->m_CommandBuffers[i]);
        	}
        	
        	m_GUI->BindDrawCmd(CommandPool->m_CommandBuffers[i],FrameBuffer->m_RenderPass);
        	
            vkCmdEndRenderPass(CommandPool->m_CommandBuffers[i]);

            if (vkEndCommandBuffer(CommandPool->m_CommandBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }
	
    void VulkanContext::CreateMeshBuffer()
    {
    	auto cmd = VulkanCommandBuffer::Create(Instance->GetDevice(),CommandPool->m_CommandPool);
    	
		Model = VulkanModel::LoadFromFile("Resources/suzanne.obj",
			Instance->m_Device,
			cmd,
			{ VertexAttribute::VA_Position, VertexAttribute::VA_Normal }); 
    }
	
    void VulkanContext::createUniformBuffer()
    {
    	ubo.model = glm::mat4(1.0f);
    	ubo.view = glm::mat4(1.0f);
    	ubo.proj = glm::mat4(1.0f);
    	
    	 UniformBuffer = VulkanBuffer::CreateBuffer(
    		Instance->GetDevice(),
    		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			sizeof(UniformBufferObject),
			&ubo);
    }
	
    void VulkanContext::UpdateUniformBuffer(Timestep ts)
    {
        ubo.model = glm::rotate(ubo.model, ts.GetSeconds() *glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));; 
        ubo.view = glm::lookAtLH(glm::vec3(0, 0, -30.0f),glm::vec3(0, 0, 0),glm::vec3(0,1.0f,0.0f));
    	ubo.proj = glm::perspectiveLH_ZO(glm::radians(45.0f), (float)WinProperty->Width / (float) WinProperty->Height, 0.1f, 1000.0f);

    	UniformBuffer->Map();
    	UniformBuffer->CopyFrom(&ubo,sizeof(UniformBufferObject));
    	UniformBuffer->UnMap();
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

    bool VulkanContext::UpdateUI(float time, float delta)
    {
    	m_GUI->StartFrame();

    	bool yes = true;
	    {
    		static float f = 0.0f;
    		static glm::vec3 color(0, 0, 0);
    		static int counter = 0;

    		bool a = false;
    		ImGui::SetNextWindowPos(ImVec2(0, 0));
    		// ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
    		ImGui::Begin("ImGUI!", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    		ImGui::Checkbox("AutoRotate", &a);

    		ImGui::Text("This is some useful text.");
    		ImGui::Checkbox("Demo Window", &yes);
    		ImGui::Checkbox("Another Window", &yes);

    		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    		ImGui::ColorEdit3("clear color", (float*)&color);

    		if (ImGui::Button("Button"))
    		{
    			counter++;
    		}

    		ImGui::SameLine();
    		ImGui::Text("counter = %d", counter);

    		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    		ImGui::End();
	    }

    	// bool hovered = ImGui::IsAnyWindowHovered() || ImGui::IsAnyItemHovered() || ImGui::IsRootWindowOrAnyChildHovered();

    	m_GUI->EndFrame();

    	if (m_GUI->Update())
    	{
    		CreateCommandBuffers();
    	}

    	return false;
    }

    void VulkanContext::CreateDescriptorPool()
    {
        VkDescriptorPoolSize poolSize = {};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = 1;

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = 1;

        if (vkCreateDescriptorPool(Instance->GetDevice()->GetInstanceHandle(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void VulkanContext::CreateDescriptorSet()
    {
        VkDescriptorSetLayout layouts[] = {descriptorSetLayout};
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = layouts;

        if (vkAllocateDescriptorSets(Instance->GetDevice()->GetInstanceHandle(), &allocInfo, &descriptorSet) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor set!");
        }

        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = UniformBuffer->Buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(Instance->GetDevice()->GetInstanceHandle(), 1, &descriptorWrite, 0, nullptr);
    }

    void VulkanContext::CreateDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding = {};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;

        if (vkCreateDescriptorSetLayout(Instance->GetDevice()->GetInstanceHandle(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

}

