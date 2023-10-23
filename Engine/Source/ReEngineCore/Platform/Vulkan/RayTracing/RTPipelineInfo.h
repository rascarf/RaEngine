#pragma once
#include "SBTWrapper.h"
#include "Core/Core.h"
#include "Platform/Vulkan/VulkanCommonDefine.h"
#include "Platform/Vulkan/VulkanShader/VulkanShader.h"
#include "Platform/Vulkan/VulkanShader/VulkanShaderModule.h"

namespace ReEngine
{
    struct dim3 {
        uint32_t x = 1;
        uint32_t y = 1;
        uint32_t z = 1;
    };
    
    class RTPipelineInfo
    {
    public:

        RTPipelineInfo(){}
        ~RTPipelineInfo(){}

        void Destory();
    
    public:
        void CreatePipeline(
            Ref<VulkanShader> RayTracingShader,
            uint32_t RecursionDepth,
            std::vector<uint32_t> SpecializationData,
            dim3 Dims,
            VkAccelerationStructureKHR Accel
        );
    
    public:
        VkPipeline RayTracingPipeline = VK_NULL_HANDLE;
        SBTWrapper ShaderBindingTableWrapper;
        uint32_t PushConstantSize = 0;

        Ref<VulkanDevice> mDevice;
        Ref<VulkanDescriptorSet> RayTracingSet;
    };

}
