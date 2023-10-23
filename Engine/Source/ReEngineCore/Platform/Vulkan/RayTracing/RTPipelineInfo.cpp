#include "RTPipelineInfo.h"

#include "Renderer/RHI/Renderer.h"


void RTPipelineInfo::Destory()
{
    RayTracingSet.reset();
    vkDestroyPipeline(mDevice->GetInstanceHandle(),RayTracingPipeline,nullptr);
    ShaderBindingTableWrapper.Destroy();
}

void RTPipelineInfo::CreatePipeline(Ref<VulkanShader> RayTracingShader, uint32_t RecursionDepth,std::vector<uint32_t> SpecializationData, dim3 Dims, VkAccelerationStructureKHR Accel)
{
    RayTracingSet = RayTracingShader->AllocateDescriptorSet();
    RayTracingSet->WriteAccelerationStruct("topLevelAS",Accel);
    
    std::vector<VkPipelineShaderStageCreateInfo> stages;
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;

    int stage_idx = 0;
    VkPipelineShaderStageCreateInfo stage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    stage.pName = "main";

    // RayGeneration
    VkRayTracingShaderGroupCreateInfoKHR group{VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
    group.anyHitShader = VK_SHADER_UNUSED_KHR;
    group.closestHitShader = VK_SHADER_UNUSED_KHR;
    group.generalShader = VK_SHADER_UNUSED_KHR; 
    group.intersectionShader = VK_SHADER_UNUSED_KHR;
    
    stage.module = RayTracingShader->RayGenerationShaderModule->Handle;
    stage.stage = RayTracingShader->RayGenerationShaderModule->mShaderStage;
    group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group.generalShader = stage_idx;
    
    groups.push_back(group);
    stages.push_back(stage);
    stage_idx++; 

    // ClosestHit
    group.anyHitShader = VK_SHADER_UNUSED_KHR;
    group.closestHitShader = VK_SHADER_UNUSED_KHR;
    group.generalShader = VK_SHADER_UNUSED_KHR;
    group.intersectionShader = VK_SHADER_UNUSED_KHR;
    
    stage.module = RayTracingShader->ClosethitShaderModule->Handle;
    stage.stage = RayTracingShader->ClosethitShaderModule->mShaderStage;
    group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
    group.closestHitShader = stage_idx;
    
    groups.push_back(group);
    stages.push_back(stage);
    stage_idx++;

    // Miss
    group.anyHitShader = VK_SHADER_UNUSED_KHR; 
    group.closestHitShader = VK_SHADER_UNUSED_KHR;
    group.generalShader = VK_SHADER_UNUSED_KHR;
    group.intersectionShader = VK_SHADER_UNUSED_KHR;
    
    stage.module = RayTracingShader->MissShaderModule->Handle;
    stage.stage = RayTracingShader->MissShaderModule->mShaderStage;
    group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group.generalShader = stage_idx;
    
    groups.push_back(group);
    stages.push_back(stage);
    stage_idx++;
    
    VkRayTracingPipelineCreateInfoKHR Pipeline{VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR};
    Pipeline.stageCount = static_cast<uint32_t>(stages.size());
    Pipeline.pStages = stages.data();
    Pipeline.groupCount = static_cast<uint32_t>(groups.size());
    Pipeline.pGroups = groups.data();
    Pipeline.maxPipelineRayRecursionDepth = 16;// todo 暴露出去
    Pipeline.layout = RayTracingShader->pipelineLayout;
    Pipeline.flags = 0; 
    vkCreateRayTracingPipelinesKHR(mDevice->GetInstanceHandle(),{},{},1,&Pipeline,nullptr,&RayTracingPipeline);

    auto Context = Renderer::GetContext().get();
    auto VkContext = dynamic_cast<ReEngine::VulkanContext*>(Context);

    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};
    VkPhysicalDeviceProperties2 PhysicalDeviceProperties;
    PhysicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    PhysicalDeviceProperties.pNext = &rtProperties;
    vkGetPhysicalDeviceProperties2(mDevice->GetPhysicalHandle(), &PhysicalDeviceProperties);
    
    ShaderBindingTableWrapper.Setup(VkContext,mDevice->GetGraphicsQueue()->GetFamilyIndex(),rtProperties);
    ShaderBindingTableWrapper.Create(RayTracingPipeline,Pipeline);
}
