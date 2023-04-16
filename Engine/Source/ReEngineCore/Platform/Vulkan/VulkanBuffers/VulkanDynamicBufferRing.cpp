#include "VulkanDynamicBufferRing.h"

VkResult VulkanDynamicBufferRing::OnCreate(Ref<VulkanDevice> Device, uint32_t NUmberOfBackBuffers,uint32_t MemTotalSize, char* name)
{
    m_Device = Device;

    m_MemTotalSize = AlignUp(MemTotalSize,256u);

    m_Mem.OnCreate(NUmberOfBackBuffers,m_MemTotalSize);

    m_Buffer = VulkanBuffer::CreateBuffer(
        Device,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_MemTotalSize
    );
    
    m_Buffer->Map();
    m_pData = (char*)m_Buffer->Mapped;

    return VK_SUCCESS;
}

void VulkanDynamicBufferRing::OnDestroy()
{
    m_Mem.OnDestroy();
}

bool VulkanDynamicBufferRing::AllocConstantBuffer(uint32_t size, void** pData, VkDescriptorBufferInfo* pOut)
{
    size = AlignUp(size, 256u);

    uint32_t MemOffset;
    if(m_Mem.Alloc(size,&MemOffset) == false)
    {
        RE_CORE_ERROR("Ran out of mem for 'dynamic' buffers, please increase the allocated size");
        return false;
    }

    *pData = (void*)(m_pData + MemOffset);

    pOut->buffer = m_Buffer->Buffer;
    pOut->offset = MemOffset;
    pOut->range = size;

    return true;
}

VkDescriptorBufferInfo VulkanDynamicBufferRing::AllocConstantBuffer(uint32_t size, void* pData)
{
    void *pBuffer;
    VkDescriptorBufferInfo out;
    if (AllocConstantBuffer(size, &pBuffer, &out))
    {
        memcpy(pBuffer, pData, size);
    }

    return out;
}

void VulkanDynamicBufferRing::OnBeginFrame()
{
    m_Mem.OnBeginFrame();
}

void VulkanDynamicBufferRing::SetDescriptorSet(int BindingIndex, uint32_t size, VkDescriptorSet descriptorSet)
{
    VkDescriptorBufferInfo out = {};
    out.buffer = m_Buffer->Buffer;
    out.offset = 0;
    out.range = size;

    VkWriteDescriptorSet write;
    write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = NULL;
    write.dstSet = descriptorSet;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    write.pBufferInfo = &out;
    write.dstArrayElement = 0;
    write.dstBinding = BindingIndex;

    vkUpdateDescriptorSets(m_Device->GetInstanceHandle(), 1, &write, 0, NULL);
}
