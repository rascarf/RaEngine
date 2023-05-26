#pragma once
#include "Platform/Vulkan/VulkanCommonDefine.h"
#include "Platform/Vulkan/VulkanDevice.h"

class VulkanDescriptorSetLayoutsInfo;

class VulkanDescriptorAllocator
{
public:
    void OnCreate(Ref<VulkanDevice> Device,uint32 UniformBufferCount,uint32 SrvDescriptorCount,uint32 UavDescriptorCount,uint32_t SamplerDescriptorCount);
    void OnDestroy();

    bool AllocDescriptor(VkDescriptorSetLayout descriptorLayout, VkDescriptorSet *pDescriptor);
    bool AllocDescriptor(int size, const VkSampler *pSamplers, VkDescriptorSetLayout *descriptorLayout, VkDescriptorSet *pDescriptor);
    bool AllocDescriptor(std::vector<uint32_t> &descriptorCounts, const VkSampler* pSamplers, VkDescriptorSetLayout* descriptorLayout, VkDescriptorSet* pDescriptor);

    bool CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding> *pDescriptorLayoutBinding, VkDescriptorSetLayout *pDescSetLayout);
    bool CreateDescriptorSetLayoutAndAllocDescriptorSet(std::vector<VkDescriptorSetLayoutBinding> *pDescriptorLayoutBinding, VkDescriptorSetLayout *descriptorLayout, VkDescriptorSet *pDescriptor);
    void FreeDescriptor(VkDescriptorSet descriptorSet);

private:
    Ref<VulkanDevice> mDevice;
    VkDescriptorPool mDescriptorPool;
    int mAllocatedDescriptorCount = 0;
};


class VulkanShaderDescriptorPool
{
public:
    VulkanShaderDescriptorPool(VkDevice Device,uint32 inMax, const VulkanDescriptorSetLayoutsInfo& setLayoutsInfo, const std::vector<VkDescriptorSetLayout>& inDescriptorSetLayouts);

    ~VulkanShaderDescriptorPool();

    bool IsFull()
    {
        return usedSet >= maxSet;
    }

    bool AllocateDescriptorSet(VkDescriptorSet* descriptorSet)
    {
        if (usedSet + descriptorSetLayouts.size() >= maxSet)
        {
            return false;
        }

        usedSet += (int32)descriptorSetLayouts.size();

        VkDescriptorSetAllocateInfo allocInfo;
        ZeroVulkanStruct(allocInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);
        allocInfo.descriptorPool     = descriptorPool;
        allocInfo.descriptorSetCount = (uint32_t)descriptorSetLayouts.size();
        allocInfo.pSetLayouts        = descriptorSetLayouts.data();
        
        VERIFYVULKANRESULT(vkAllocateDescriptorSets(device, &allocInfo, descriptorSet));

        return true;
    }
public:
    int32                               maxSet;
    int32                               usedSet;
    VkDevice                            device = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout>  descriptorSetLayouts;
    VkDescriptorPool                    descriptorPool = VK_NULL_HANDLE;
};