#include "VulkanContext.h"

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
    	Camera = CreateRef<EditorCamera>();
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

    	m_GUI->Destroy();
    	
    	Model.reset();
    	UniformBuffer.reset();
    	ParamBuffer.reset();
    	TexCurve.reset();
    	TexDiffuse.reset();
    	TexNomal.reset();
    	TexPreIntegareted.reset();
    	GraphicsPipeline.reset();
    	
        vkDestroyDescriptorPool(Device, descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(Device, descriptorSetLayout, nullptr);
    	vkDestroyPipelineLayout(Device,pipelineLayout,nullptr);
    	
    	FrameBuffer->ShutDown();
    	CommandPool->ShutDown();
        Instance->Shutdown();
    }

    void VulkanContext::SwapBuffers(Timestep ts)
    {
    	UpdateUI(ts.GetMilliseconds(),ts.GetSeconds());
        UpdateUniformBuffer(ts);
    	
  		const int32 bufferIndex = CommandPool->AcquireBackbufferIndex();
		CommandPool->Present(bufferIndex < 0 ? 0 : bufferIndex);
    }

    void VulkanContext::RecreateSwapChain()
    {
    	const auto Device = Instance->GetDevice()->GetInstanceHandle();
    	
    	FrameBuffer->ShutDown();
    	CommandPool->ShutDown();
    	
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

    void VulkanContext::CreateGraphicsPipeline()
    {
        VkShaderModule vertShaderModule = CreateShaderModule(SHADER_VERT);
		VkShaderModule fragShaderModule = CreateShaderModule(SHADER_FRAG);
    	
    	VulkanPipelineInfo DefaultInfo;
    	DefaultInfo.VertShaderModule = vertShaderModule;
    	DefaultInfo.FragShaderModule = fragShaderModule;
    	
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

		if (vkCreatePipelineLayout(Instance->GetDevice()->GetInstanceHandle(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!"); 
		}

    	auto VertexInputBinding = Model->GetInputBinding();
    	auto AttributeInputBinding = Model->GetInputAttributes();

    	GraphicsPipeline = VulkanPipeline::Create(
			Instance->GetDevice(),
			CommandPool->m_PipelineCache,
			DefaultInfo,
			{VertexInputBinding},
			AttributeInputBinding,
			pipelineLayout,
			FrameBuffer->m_RenderPass
    	);

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

        	VkViewport viewport = {};
        	viewport.x        = 0;
        	viewport.y        = WinProperty->Height;
        	viewport.width    = WinProperty->Width;
        	viewport.height   = -(float)WinProperty->Height;    // flip y axis
        	viewport.minDepth = 0.0f;
        	viewport.maxDepth = 1.0f;
		
        	VkRect2D scissor = {};
        	scissor.extent.width  = WinProperty->Width;
        	scissor.extent.height = WinProperty->Height;
        	scissor.offset.x      = 0;
        	scissor.offset.y      = 0;

        	vkCmdSetViewport(CommandPool->m_CommandBuffers[i], 0, 1, &viewport);
        	vkCmdSetScissor(CommandPool->m_CommandBuffers[i], 0, 1, &scissor);
        	
            vkCmdBindPipeline(CommandPool->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline->Pipeline);
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
    	
		Model = VulkanModel::LoadFromFile("Assets/Mesh/head.obj",
			Instance->m_Device,
			cmd,
			{ VertexAttribute::VA_Position, VertexAttribute::VA_UV0, VertexAttribute::VA_Normal, VertexAttribute::VA_Tangent});

		TexDiffuse = VulkanTexture::Create2D(
		"Assets/Textures/head_diffuse.jpg",Instance->m_Device,cmd
		);
    	
    	TexPreIntegareted = VulkanTexture::Create2D(
"Assets/Textures/preIntegratedLUT.png",Instance->m_Device,cmd
		);
    	
    	TexCurve = VulkanTexture::Create2D(
"Assets/Textures/curvatureLUT.png",Instance->m_Device,cmd
		);

    	TexNomal = VulkanTexture::Create2D(
		"Assets/Textures/head_normal.jpg",Instance->m_Device,cmd
		);
    	
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

    	Param.blurredLevel = 2.0;
    	Param.curvature = 3.5;
    	Param.curvatureScaleBias.x = 0.101;
    	Param.curvatureScaleBias.y = -0.001;
    	Param.exposure = 1.0;
    	Param.lightColor = glm::vec3(240.0f / 255.0f, 200.0f / 255.0f, 166.0f / 255.0f);
    	Param.lightDir = glm::vec3(1, 0, -1.0);
    	Param.lightDir = normalize(Param.lightDir);
    	Param.padding = 0.0;

    	ParamBuffer = VulkanBuffer::CreateBuffer(
    		Instance->GetDevice(),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			sizeof(ParamBlock),
			&Param);
    }
	
    void VulkanContext::UpdateUniformBuffer(Timestep ts)
    {
    	BoundingBox Box = Model->RootNode->GetBounds();

    	glm::vec3 boundSize   = Box.Max - Box.Min;
    	glm::vec3 boundCenter = Box.Min + boundSize * 0.5f;

    	ubo.model = glm::rotate(ubo.model, ts.GetSeconds() *  glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));;
        ubo.view = glm::lookAtLH(glm::vec3(boundCenter.x, boundCenter.y, boundCenter.z - 50.0f),boundCenter,glm::vec3(0,1.0f,0.0f));
    	ubo.proj = glm::perspectiveLH_ZO(glm::radians(45.0f), (float)WinProperty->Width / (float) WinProperty->Height, 0.1f, 1000.0f);

    	UniformBuffer->Map();
    	UniformBuffer->CopyFrom(&ubo,sizeof(UniformBufferObject));
    	UniformBuffer->UnMap();

    	ParamBuffer->Map();
		ParamBuffer->CopyFrom(&Param,sizeof(ParamBlock));
    	ParamBuffer->UnMap();
    }

    void VulkanContext::OnEvent(std::shared_ptr<Event> e)
    {
    	m_GUI->OnEvent(e);
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
	    {
    		ImGui::SetNextWindowPos(ImVec2(0, 0));
    		ImGui::Begin("Texture", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    		ImGui::Text("SSS Demo");

    		for (int32 i = 0; i < Model->Meshes.size(); ++i)
    		{
    			Ref<VulkanMesh> mesh = Model->Meshes[i];
    			ImGui::Text("%-20s Tri:%d", mesh->LinkNode.lock()->name.c_str(), mesh->TriangleCount);
    		}

    		ImGui::SliderFloat("Curvature", &(Param.curvature),       0.0f, 10.0f);
    		ImGui::SliderFloat2("CurvatureBias", (float*)&(Param.curvatureScaleBias), 0.0f, 1.0f);

    		ImGui::SliderFloat("BlurredLevel", &(Param.blurredLevel), 0.0f, 12.0f);
    		ImGui::SliderFloat("Exposure", &(Param.exposure),         0.0f, 10.0f);

    		ImGui::SliderFloat3("LightDirection", (float*)&(Param.lightDir), -10.0f, 10.0f);
    		ImGui::ColorEdit3("LightColor", (float*)&(Param.lightColor));

    		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    		BoundingBox Box = Model->RootNode->GetBounds();
    		glm::vec3 boundSize   = Box.Max - Box.Min;
    		glm::vec3 boundCenter = Box.Min + boundSize * 0.5f;
    		ImGui::InputFloat3("Mesh", &(boundCenter.r));
    		
    		auto CameraPostion = Camera->GetPosition();
    		ImGui::InputFloat3("CameraPosition", &(CameraPostion.r));
    		
    		ImGui::End();
	    }

    	bool hovered = ImGui::IsAnyItemHovered();
    	
    	m_GUI->EndFrame();

    	if (m_GUI->Update())
    	{
    		CreateCommandBuffers();
    	}

    	return hovered;
    }

    void VulkanContext::CreateDescriptorPool()
    {
    	VkDescriptorPoolSize poolSizes[2];
    	poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    	poolSizes[0].descriptorCount = 2;
    	poolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    	poolSizes[1].descriptorCount = 4;

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 2;
        poolInfo.pPoolSizes = poolSizes;
        poolInfo.maxSets = 1;

        if (vkCreateDescriptorPool(Instance->GetDevice()->GetInstanceHandle(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        {
            RE_CORE_ERROR("failed to create descriptor pool!");
        }
    	
    	VkDescriptorSetAllocateInfo allocInfo;
    	ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
    	allocInfo.descriptorPool     = descriptorPool;
    	allocInfo.descriptorSetCount = 1;
    	allocInfo.pSetLayouts        = &descriptorSetLayout;
    	VERIFYVULKANRESULT(vkAllocateDescriptorSets(Instance->GetDevice()->GetInstanceHandle(), &allocInfo, &descriptorSet));
    }

    void VulkanContext::CreateDescriptorSet()
    {
        VkWriteDescriptorSet descriptorWrite = {};
    	ZeroVulkanStruct(descriptorWrite, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.pBufferInfo = &(UniformBuffer->Descriptor);
        vkUpdateDescriptorSets(Instance->GetDevice()->GetInstanceHandle(), 1, &descriptorWrite, 0, nullptr);

    	ZeroVulkanStruct(descriptorWrite, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
    	descriptorWrite.dstSet          = descriptorSet;
    	descriptorWrite.descriptorCount = 1;
    	descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    	descriptorWrite.pBufferInfo     = &(ParamBuffer->Descriptor);
    	descriptorWrite.dstBinding      = 1;
    	vkUpdateDescriptorSets(Instance->GetDevice()->GetInstanceHandle(), 1, &descriptorWrite, 0, nullptr);
    	
    	std::vector<Ref<VulkanTexture>> Textures =
    	{
    		TexDiffuse,TexNomal,TexCurve,TexPreIntegareted
    	};

    	for (int32 i = 0; i < 4; ++i)
    	{
    		ZeroVulkanStruct(descriptorWrite, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
    		descriptorWrite.dstSet          = descriptorSet;
    		descriptorWrite.descriptorCount = 1;
    		descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    		descriptorWrite.pBufferInfo     = nullptr;
    		descriptorWrite.pImageInfo      = &(Textures[i]->DescriptorInfo);
    		descriptorWrite.dstBinding      = 2 + i;
    		vkUpdateDescriptorSets(Instance->GetDevice()->GetInstanceHandle(), 1, &descriptorWrite, 0, nullptr);
    	}
    }

    void VulkanContext::CreateDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding[6] = {};
        uboLayoutBinding[0].binding = 0;
        uboLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding[0].descriptorCount = 1;
        uboLayoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding[0].pImmutableSamplers = nullptr; // Optional

    	uboLayoutBinding[1].binding 			 = 1;
    	uboLayoutBinding[1].descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    	uboLayoutBinding[1].descriptorCount    = 1;
    	uboLayoutBinding[1].stageFlags 		 = VK_SHADER_STAGE_FRAGMENT_BIT;
    	uboLayoutBinding[1].pImmutableSamplers = nullptr;

    	for(int32 i = 0 ; i < 4 ;i++)
    	{
    		uboLayoutBinding[2 + i].binding = 2 + i;
    		uboLayoutBinding[2 + i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    		uboLayoutBinding[2 + i].descriptorCount    = 1;
    		uboLayoutBinding[2 + i].stageFlags 		 = VK_SHADER_STAGE_FRAGMENT_BIT;
    		uboLayoutBinding[2 + i].pImmutableSamplers = nullptr;
    	}

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 6;
        layoutInfo.pBindings = uboLayoutBinding;

        if (vkCreateDescriptorSetLayout(Instance->GetDevice()->GetInstanceHandle(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
        {
            RE_CORE_ERROR("failed to create descriptor set layout!");
        }
    }

}

