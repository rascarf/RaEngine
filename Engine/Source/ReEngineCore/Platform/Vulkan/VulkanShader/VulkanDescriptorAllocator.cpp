#include "VulkanDescriptorAllocator.h"
#include "Platform/Vulkan/VulkanShader/VulkanShader.h"

void VulkanDescriptorAllocator::OnCreate(Ref<VulkanDevice> Device, uint32 UniformBufferCount, uint32 SrvDescriptorCount,uint32 UavDescriptorCount, uint32_t SamplerDescriptorCount)
{
    mDevice = Device;
    mAllocatedDescriptorCount = 0;

    const VkDescriptorPoolSize TypeCount[] =
    {
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, UniformBufferCount},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, UniformBufferCount},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SrvDescriptorCount},
        {VK_DESCRIPTOR_TYPE_SAMPLER, SamplerDescriptorCount},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, UavDescriptorCount},
    };

    VkDescriptorPoolCreateInfo descriptor_pool = {};
    descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool.pNext = NULL;
    descriptor_pool.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    descriptor_pool.maxSets = 8000;
    descriptor_pool.poolSizeCount = _countof( TypeCount );
    descriptor_pool.pPoolSizes = TypeCount;

    VkResult res = vkCreateDescriptorPool(mDevice->GetInstanceHandle(), &descriptor_pool, NULL, &mDescriptorPool);

    assert(res == VK_SUCCESS);
}

void VulkanDescriptorAllocator::OnDestroy()
{
    vkDestroyDescriptorPool(mDevice->GetInstanceHandle(), mDescriptorPool, NULL);
}

bool VulkanDescriptorAllocator::AllocDescriptor(VkDescriptorSetLayout descriptorLayout, VkDescriptorSet* pDescriptorSet)
{
    VkDescriptorSetAllocateInfo alloc_info;
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.descriptorPool = mDescriptorPool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &descriptorLayout;

    VkResult res = vkAllocateDescriptorSets(mDevice->GetInstanceHandle(), &alloc_info, pDescriptorSet);
    assert(res == VK_SUCCESS);

    mAllocatedDescriptorCount++;

    return res == VK_SUCCESS;
}

bool VulkanDescriptorAllocator::AllocDescriptor(int size, const VkSampler *pSamplers, VkDescriptorSetLayout *pDescSetLayout, VkDescriptorSet *pDescriptorSet)
{
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings(size);
    for (int i = 0; i < size; i++)
    {
        layoutBindings[i].binding = i;
        layoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBindings[i].descriptorCount = 1;
        layoutBindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBindings[i].pImmutableSamplers = (pSamplers != NULL) ? &pSamplers[i] : NULL;
    }

    return CreateDescriptorSetLayoutAndAllocDescriptorSet(&layoutBindings, pDescSetLayout, pDescriptorSet);
}

bool VulkanDescriptorAllocator::AllocDescriptor(std::vector<uint32_t>& descriptorCounts, const VkSampler* pSamplers, VkDescriptorSetLayout* pDescSetLayout, VkDescriptorSet* pDescriptorSet)
{
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings(descriptorCounts.size());
    for (int i = 0; i < descriptorCounts.size(); i++)
    {
        layoutBindings[i].binding = i;
        layoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layoutBindings[i].descriptorCount = descriptorCounts[i];
        layoutBindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBindings[i].pImmutableSamplers = (pSamplers != NULL) ? &pSamplers[i] : NULL;
    }

    return CreateDescriptorSetLayoutAndAllocDescriptorSet(&layoutBindings, pDescSetLayout, pDescriptorSet);
}

bool VulkanDescriptorAllocator::CreateDescriptorSetLayout(std::vector<VkDescriptorSetLayoutBinding>* pDescriptorLayoutBinding, VkDescriptorSetLayout* pDescSetLayout)
{
    VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
    descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout.pNext = NULL;
    descriptor_layout.bindingCount = (uint32_t)pDescriptorLayoutBinding->size();
    descriptor_layout.pBindings = pDescriptorLayoutBinding->data();
    descriptor_layout.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

    VkResult res = vkCreateDescriptorSetLayout(mDevice->GetInstanceHandle(), &descriptor_layout, NULL, pDescSetLayout);

    assert(res == VK_SUCCESS);
    
    return (res == VK_SUCCESS);
}

bool VulkanDescriptorAllocator::CreateDescriptorSetLayoutAndAllocDescriptorSet(std::vector<VkDescriptorSetLayoutBinding>* pDescriptorLayoutBinding, VkDescriptorSetLayout *pDescSetLayout, VkDescriptorSet *pDescriptorSet)
{
    // Next take layout bindings and use them to create a descriptor set layout

    VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
    descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout.pNext = NULL;
    descriptor_layout.bindingCount = (uint32_t)pDescriptorLayoutBinding->size();
    descriptor_layout.pBindings = pDescriptorLayoutBinding->data();

    VkResult res = vkCreateDescriptorSetLayout(mDevice->GetInstanceHandle(), &descriptor_layout, NULL, pDescSetLayout);
    assert(res == VK_SUCCESS);

    return AllocDescriptor(*pDescSetLayout, pDescriptorSet);
}

void VulkanDescriptorAllocator::FreeDescriptor(VkDescriptorSet descriptorSet)
{
    mAllocatedDescriptorCount--;
    vkFreeDescriptorSets(mDevice->GetInstanceHandle(), mDescriptorPool, 1, &descriptorSet);
}

VulkanShaderDescriptorPool::VulkanShaderDescriptorPool(VkDevice Device, uint32 inMax,const VulkanDescriptorSetLayoutsInfo& setLayoutsInfo,const std::vector<VkDescriptorSetLayout>& inDescriptorSetLayouts)
{
    device = Device;
    maxSet = inMax;
    usedSet = 0;
    descriptorSetLayouts = inDescriptorSetLayouts;

    std::vector<VkDescriptorPoolSize> poolSizes;
    for (int32 i = 0; i < setLayoutsInfo.SetLayouts.size(); ++i)
    {
        const VulkanDescriptorSetLayoutInfo& setLayoutInfo = setLayoutsInfo.SetLayouts[i];
        for (int32 j = 0; j < setLayoutInfo.Bindings.size(); ++j)
        {
            VkDescriptorPoolSize poolSize = {};
            poolSize.type            = setLayoutInfo.Bindings[j].descriptorType;
            poolSize.descriptorCount = setLayoutInfo.Bindings[j].descriptorCount;
            poolSizes.push_back(poolSize);
        }
    }

    VkDescriptorPoolCreateInfo descriptorPoolInfo;
    ZeroVulkanStruct(descriptorPoolInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
    descriptorPoolInfo.poolSizeCount = (uint32_t)poolSizes.size();
    descriptorPoolInfo.pPoolSizes    = poolSizes.data();
    descriptorPoolInfo.maxSets       = maxSet;
    descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    VERIFYVULKANRESULT(vkCreateDescriptorPool(device, &descriptorPoolInfo, VULKAN_CPU_ALLOCATOR, &descriptorPool));
}

VulkanShaderDescriptorPool::~VulkanShaderDescriptorPool()
{
    if (descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(device, descriptorPool, VULKAN_CPU_ALLOCATOR);
        descriptorPool = VK_NULL_HANDLE;
    }   
}
