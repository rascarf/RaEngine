#include "VulkanShaderModule.h"

Ref<VulkanShaderModule> VulkanShaderModule::Create(Ref<VulkanDevice> vulkanDevice,const std::vector<unsigned char>& code,VkShaderStageFlagBits shaderStage)
{
    VkDevice Device = vulkanDevice->GetInstanceHandle();
    
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        RE_CORE_ERROR("failed to create shader module!");
    }

    Ref<VulkanShaderModule> vulkanShaderModule = CreateRef<VulkanShaderModule>();
    vulkanShaderModule->Code = &code;
    vulkanShaderModule->Handle = shaderModule;
    vulkanShaderModule->mShaderStage = shaderStage;
    vulkanShaderModule->mDevice = Device;

    return vulkanShaderModule;
}
