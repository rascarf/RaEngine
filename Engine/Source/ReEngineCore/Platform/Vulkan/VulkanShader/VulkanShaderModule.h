#pragma once
#include "Platform/Vulkan/VulkanCommonDefine.h"
#include "Platform/Vulkan/VulkanDevice.h"

class VulkanShaderModule
{
public:
    VulkanShaderModule() = default;
    virtual ~VulkanShaderModule()
    {
        if (Handle != VK_NULL_HANDLE) 
        {
            vkDestroyShaderModule(mDevice, Handle, VULKAN_CPU_ALLOCATOR);
            Handle = VK_NULL_HANDLE;
        }
    }

    static Ref<VulkanShaderModule> Create(Ref<VulkanDevice> vulkanDevice,const std::vector<unsigned char>& code,VkShaderStageFlagBits shaderStage);
    
public:
    
    VkDevice mDevice;
    VkShaderStageFlagBits mShaderStage;
    VkShaderModule Handle;
    const std::vector<unsigned char>* Code = nullptr;
};
