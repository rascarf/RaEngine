#pragma once
#include "VulkanCommonDefine.h"
#include "VulkanDevice.h"
#include "VulkanShader/VulkanShader.h"

struct VulkanPipelineInfo
{
    VkPipelineInputAssemblyStateCreateInfo		InputAssemblyState;
    VkPipelineRasterizationStateCreateInfo		RasterizationState;
    VkPipelineColorBlendAttachmentState			BlendAttachmentStates[8];
    VkPipelineDepthStencilStateCreateInfo		DepthStencilState;
    VkPipelineMultisampleStateCreateInfo		MultisampleState;

    VkShaderModule	VertShaderModule = VK_NULL_HANDLE;
    VkShaderModule	FragShaderModule = VK_NULL_HANDLE;
    VkShaderModule	CompShaderModule = VK_NULL_HANDLE;
    VkShaderModule	TescShaderModule = VK_NULL_HANDLE;
    VkShaderModule	TeseShaderModule = VK_NULL_HANDLE;
    VkShaderModule	GeomShaderModule = VK_NULL_HANDLE;

    //TODO Raytracing Shader
	Ref<VulkanShader> Shader;
    int32 SubPass = 0;
    int32 ColorAttachMenst = 1;
    
    VulkanPipelineInfo()
    {
        ZeroVulkanStruct(InputAssemblyState, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
        InputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		
        ZeroVulkanStruct(RasterizationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
        RasterizationState.polygonMode 			   = VK_POLYGON_MODE_FILL;
        RasterizationState.cullMode                = VK_CULL_MODE_BACK_BIT;
        RasterizationState.frontFace               = VK_FRONT_FACE_CLOCKWISE;
        RasterizationState.depthClampEnable        = VK_FALSE;
        RasterizationState.rasterizerDiscardEnable = VK_FALSE;
        RasterizationState.depthBiasEnable         = VK_FALSE;
        RasterizationState.lineWidth 			   = 1.0f;


        for (int32 i = 0; i < 8; ++i)
        {
            BlendAttachmentStates[i] = {};
            BlendAttachmentStates[i].colorWriteMask = (
                VK_COLOR_COMPONENT_R_BIT |
                VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT |
                VK_COLOR_COMPONENT_A_BIT
            );
            
            BlendAttachmentStates[i].blendEnable = VK_FALSE;
            BlendAttachmentStates[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            BlendAttachmentStates[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
            BlendAttachmentStates[i].colorBlendOp        = VK_BLEND_OP_ADD;
            BlendAttachmentStates[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            BlendAttachmentStates[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            BlendAttachmentStates[i].alphaBlendOp        = VK_BLEND_OP_ADD;
        }
        
        ZeroVulkanStruct(DepthStencilState, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
        DepthStencilState.depthTestEnable 		= VK_TRUE;
        DepthStencilState.depthWriteEnable 		= VK_TRUE;
        DepthStencilState.depthCompareOp		= VK_COMPARE_OP_LESS_OR_EQUAL;
        DepthStencilState.depthBoundsTestEnable = VK_FALSE;
        DepthStencilState.back.failOp 			= VK_STENCIL_OP_KEEP;
        DepthStencilState.back.passOp 			= VK_STENCIL_OP_KEEP;
        DepthStencilState.back.compareOp 		= VK_COMPARE_OP_ALWAYS;
        DepthStencilState.stencilTestEnable 	= VK_TRUE;
        DepthStencilState.front 				= DepthStencilState.back;

        ZeroVulkanStruct(MultisampleState, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
        MultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        MultisampleState.pSampleMask 		  = nullptr;
    }

    void FillShaderStages(std::vector<VkPipelineShaderStageCreateInfo>& shaderStages)
	{
		if (VertShaderModule != VK_NULL_HANDLE) {
			VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
			ZeroVulkanStruct(shaderStageCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
			shaderStageCreateInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
			shaderStageCreateInfo.module = VertShaderModule;
			shaderStageCreateInfo.pName  = "main";
			shaderStages.push_back(shaderStageCreateInfo);
		}

		if (FragShaderModule != VK_NULL_HANDLE) {
			VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
			ZeroVulkanStruct(shaderStageCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
			shaderStageCreateInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
			shaderStageCreateInfo.module = FragShaderModule;
			shaderStageCreateInfo.pName  = "main";
			shaderStages.push_back(shaderStageCreateInfo);
		}

		if (CompShaderModule != VK_NULL_HANDLE) {
			VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
			ZeroVulkanStruct(shaderStageCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
			shaderStageCreateInfo.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
			shaderStageCreateInfo.module = CompShaderModule;
			shaderStageCreateInfo.pName  = "main";
			shaderStages.push_back(shaderStageCreateInfo);
		}

		if (GeomShaderModule != VK_NULL_HANDLE) {
			VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
			ZeroVulkanStruct(shaderStageCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
			shaderStageCreateInfo.stage  = VK_SHADER_STAGE_GEOMETRY_BIT;
			shaderStageCreateInfo.module = GeomShaderModule;
			shaderStageCreateInfo.pName  = "main";
			shaderStages.push_back(shaderStageCreateInfo);
		}

		if (TescShaderModule != VK_NULL_HANDLE) {
			VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
			ZeroVulkanStruct(shaderStageCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
			shaderStageCreateInfo.stage  = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			shaderStageCreateInfo.module = TescShaderModule;
			shaderStageCreateInfo.pName  = "main";
			shaderStages.push_back(shaderStageCreateInfo);
		}

		if (TeseShaderModule != VK_NULL_HANDLE) {
			VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
			ZeroVulkanStruct(shaderStageCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
			shaderStageCreateInfo.stage  = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			shaderStageCreateInfo.module = TeseShaderModule;
			shaderStageCreateInfo.pName  = "main";
			shaderStages.push_back(shaderStageCreateInfo);
		}
	}
};


class VulkanPipeline
{
public:
	VulkanPipeline()
		: VulkanDeviceRef(nullptr)
		, Pipeline(VK_NULL_HANDLE)
	{

	}
	
	~VulkanPipeline()
	{
		VkDevice device = VulkanDeviceRef->GetInstanceHandle();
		if (Pipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(device, Pipeline, VULKAN_CPU_ALLOCATOR);
		}
	}

	static Ref<VulkanPipeline> Create(
		Ref<VulkanDevice> VulkanDeviceRef,
		VkPipelineCache PipelineCache,
		VulkanPipelineInfo& PipelineInfo, 
		const std::vector<VkVertexInputBindingDescription>& InputBindings, 
		const std::vector<VkVertexInputAttributeDescription>& VertexInputAttributs,
		VkPipelineLayout PipelineLayout,
		VkRenderPass RenderPass
	);

public:
	Ref<VulkanDevice> VulkanDeviceRef;
	VkPipeline Pipeline;
	VkPipelineLayout PipelineLayout;
};