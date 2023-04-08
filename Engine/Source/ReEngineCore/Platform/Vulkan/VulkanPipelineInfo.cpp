#include "VulkanPipelineInfo.h"

Ref<VulkanPipeline> VulkanPipeline::Create(Ref<VulkanDevice> VulkanDeviceRef, VkPipelineCache PipelineCache,VulkanPipelineInfo& PipelineInfo, const std::vector<VkVertexInputBindingDescription>& InputBindings,const std::vector<VkVertexInputAttributeDescription>& VertexInputAttributs, VkPipelineLayout PipelineLayout,VkRenderPass RenderPass)
{
    Ref<VulkanPipeline>  pipeline   = CreateRef<VulkanPipeline>();
		pipeline->VulkanDeviceRef   = VulkanDeviceRef;
		pipeline->PipelineLayout = PipelineLayout;

		VkDevice device = VulkanDeviceRef->GetInstanceHandle();

		//组装Vertex和VertexAttribute
		VkPipelineVertexInputStateCreateInfo vertexInputState;
		ZeroVulkanStruct(vertexInputState, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
		vertexInputState.vertexBindingDescriptionCount   = InputBindings.size();
		vertexInputState.pVertexBindingDescriptions      = InputBindings.data();
		vertexInputState.vertexAttributeDescriptionCount = VertexInputAttributs.size();
		vertexInputState.pVertexAttributeDescriptions    = VertexInputAttributs.data();

		VkPipelineColorBlendStateCreateInfo colorBlendState;
		ZeroVulkanStruct(colorBlendState, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
		colorBlendState.attachmentCount = PipelineInfo.ColorAttachMenst;
		colorBlendState.pAttachments    = PipelineInfo.BlendAttachmentStates;
		
		VkPipelineViewportStateCreateInfo viewportState;
		ZeroVulkanStruct(viewportState, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
		viewportState.viewportCount = 1;
		viewportState.scissorCount  = 1;
			
		std::vector<VkDynamicState> dynamicStateEnables;
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

		VkPipelineDynamicStateCreateInfo dynamicState;
		ZeroVulkanStruct(dynamicState, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates    = dynamicStateEnables.data();

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		// if (PipelineInfo.shader) //TODO 这里使用Shader反射
		// {
		// 	shaderStages = PipelineInfo.shader->shaderStageCreateInfos;
		// }
		// else
		// {
			PipelineInfo.FillShaderStages(shaderStages);
		// }
		
		VkGraphicsPipelineCreateInfo pipelineCreateInfo;
		ZeroVulkanStruct(pipelineCreateInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
		pipelineCreateInfo.layout 				= PipelineLayout;
		pipelineCreateInfo.renderPass 			= RenderPass;
		pipelineCreateInfo.subpass              = PipelineInfo.SubPass;
		pipelineCreateInfo.stageCount 			= shaderStages.size();
		pipelineCreateInfo.pStages 				= shaderStages.data();
		pipelineCreateInfo.pVertexInputState 	= &vertexInputState;
		pipelineCreateInfo.pInputAssemblyState 	= &(PipelineInfo.InputAssemblyState);
		pipelineCreateInfo.pRasterizationState 	= &(PipelineInfo.RasterizationState);
		pipelineCreateInfo.pColorBlendState 	= &colorBlendState;
		pipelineCreateInfo.pMultisampleState 	= &(PipelineInfo.MultisampleState);
		pipelineCreateInfo.pViewportState 		= &viewportState;
		pipelineCreateInfo.pDepthStencilState 	= &(PipelineInfo.DepthStencilState);
		pipelineCreateInfo.pDynamicState 		= &dynamicState;
		VERIFYVULKANRESULT(vkCreateGraphicsPipelines(device, PipelineCache, 1, &pipelineCreateInfo, VULKAN_CPU_ALLOCATOR, &(pipeline->Pipeline)));
		
		return pipeline;
}
