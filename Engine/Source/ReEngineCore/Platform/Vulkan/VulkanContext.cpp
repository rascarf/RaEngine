#include "VulkanContext.h"

#include <array>
#include <set>
#include <limits>
#include <fstream>
#include <memory>

#include "glm/gtx/transform.hpp"
#include "Resource/AssetManager/AssetManager.h"

#include "Shader_vert.h"
#include "Shader_frag.h"
#include "VulkanBuffer.h"
#include "VulkanCommandPool.h"
#include "VulkanInstance.h"

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

    	CreateRenderPass();
    	CreateFrameBuffer();
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
       const  auto Device = Instance->GetDevice()->GetInstanceHandle();
        
        vkDeviceWaitIdle(Device);
        
        vkDestroyDescriptorPool(Device, descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(Device, descriptorSetLayout, nullptr);

    	delete VertexBuffer;
    	delete IndexBuffer;
    	delete UniformBuffer;
    	
    	CommandPool->ShutDown();
        Instance->Shutdown();
    }

    void VulkanContext::SwapBuffers()
    {
        UpdateUniformBuffer();
  		int32 bufferIndex = CommandPool->AcquireBackbufferIndex();
		CommandPool->Present(bufferIndex);
    }

    struct Vertex {
        glm::vec2 pos;
        glm::vec3 color;

        static VkVertexInputBindingDescription getBindingDescription()
        {
            VkVertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                
            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
        {
            std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);
            
            return attributeDescriptions;
        }
    };

    const std::vector<ReEngine::Vertex> vertices = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };

    //根据点的个数，可以使用uint16_：65535
    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
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

        auto BindDescription = Vertex::getBindingDescription();
        auto AttributeDescription = Vertex::getAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(AttributeDescription.size());
        vertexInputInfo.pVertexBindingDescriptions = &BindDescription;
        vertexInputInfo.pVertexAttributeDescriptions = AttributeDescription.data();

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
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(Instance->GetDevice()->GetInstanceHandle(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		vkDestroyShaderModule(Instance->GetDevice()->GetInstanceHandle(), fragShaderModule, nullptr);
		vkDestroyShaderModule(Instance->GetDevice()->GetInstanceHandle(), vertShaderModule, nullptr);
    }

    void VulkanContext::CreateDepthStencil()
    {
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

    void VulkanContext::CreateFrameBuffer()
    {
    	swapChainFramebuffers.resize(Instance->GetBackbufferImages().size());
    	for (size_t i = 0; i < Instance->GetBackbufferViews().size(); i++)
    	{
    		VkImageView attachments[] =
    		{
    			Instance->GetBackbufferViews()[i]
			};

    		VkFramebufferCreateInfo framebufferInfo = {};
    		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    		framebufferInfo.renderPass = renderPass;
    		framebufferInfo.attachmentCount = 1;
    		framebufferInfo.pAttachments = attachments;
    		framebufferInfo.width = Instance->GetSwapChain()->GetWidth();
    		framebufferInfo.height = Instance->GetSwapChain()->GetHeight();
    		framebufferInfo.layers = 1;

    		if (vkCreateFramebuffer(Instance->GetDevice()->GetInstanceHandle(), &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
    		{
    			RE_CORE_ERROR("failed to create framebuffer!");
    		}
    	}
    }

    void VulkanContext::CreateRenderPass()
    {
        VkAttachmentDescription colorAttachment = {};
        colorAttachment.format = Instance->GetSwapChain()->GetColorFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef = {};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        if (vkCreateRenderPass(Instance->GetDevice()->GetInstanceHandle(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
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
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = swapChainFramebuffers[i];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = VkExtent2D(Instance->GetSwapChain()->GetWidth(),Instance->GetSwapChain()->GetHeight());

            VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(CommandPool->m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(CommandPool->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

            VkBuffer vertexBuffers[] = {VertexBuffer->Buffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(CommandPool->m_CommandBuffers[i], 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(CommandPool->m_CommandBuffers[i],IndexBuffer->Buffer,0,VK_INDEX_TYPE_UINT16); 
            vkCmdBindDescriptorSets(CommandPool->m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
            vkCmdDrawIndexed(CommandPool->m_CommandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
            vkCmdEndRenderPass(CommandPool->m_CommandBuffers[i]);

            if (vkEndCommandBuffer(CommandPool->m_CommandBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }
	
	
    void VulkanContext::CreateMeshBuffer()
    {
        VkDeviceSize VertexbufferSize = sizeof(vertices[0]) * vertices.size();
    	VkDeviceSize IndexbufferSize = sizeof(indices[0]) * indices.size();
    	

        VulkanBuffer* stagingVertexBuffer = VulkanBuffer::CreateBuffer(
        	Instance->GetDevice(),
	        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	        VertexbufferSize,(void*)vertices.data());

    	VulkanBuffer* stagingIndexBuffer = VulkanBuffer::CreateBuffer(
			Instance->GetDevice(),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			IndexbufferSize,(void*)indices.data());
    	
    	VertexBuffer = VulkanBuffer::CreateBuffer(
			Instance->GetDevice(),
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VertexbufferSize);

    	IndexBuffer = VulkanBuffer::CreateBuffer(
			Instance->GetDevice(),
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ,
			IndexbufferSize);

    	
        //拷贝StagingBuffer到LocalBuffer
    	VulkanBuffer::TransferBuffer(*Instance,*CommandPool,stagingVertexBuffer,VertexBuffer,VertexbufferSize);
    	VulkanBuffer::TransferBuffer(*Instance,*CommandPool,stagingIndexBuffer,IndexBuffer,IndexbufferSize);
    	
    	delete stagingVertexBuffer;
        delete stagingIndexBuffer;
    }
	
    void VulkanContext::createUniformBuffer()
    {
    	static auto startTime = std::chrono::high_resolution_clock::now();
    	auto currentTime = std::chrono::high_resolution_clock::now();
    	float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;
    	
    	UniformBufferObject ubo = {};
    	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    	ubo.proj = glm::perspective(glm::radians(45.0f), WinProperty->Width / (float) WinProperty->Height, 0.1f, 10.0f);
    	ubo.proj[1][1] *= -1;
    	
    	 UniformBuffer = VulkanBuffer::CreateBuffer(
    		Instance->GetDevice(),
    		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
			sizeof(UniformBufferObject),
			&ubo);

    	UniformBuffer->Map();
    }
	

    void VulkanContext::UpdateUniformBuffer()
    {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

        UniformBufferObject ubo = {};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), WinProperty->Width / (float) WinProperty->Height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;
    	
    	UniformBuffer->CopyFrom(&ubo,sizeof(UniformBufferObject));
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

