#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"

Ref<VulkanBuffer> VulkanBuffer::CreateBuffer(std::shared_ptr<VulkanDevice> device, VkBufferUsageFlags usageFlags,
                                         VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data)
{
    Ref<VulkanBuffer> dvkBuffer = CreateRef<VulkanBuffer>();
    VkDevice vkDevice = device->GetInstanceHandle();
    dvkBuffer->Device = vkDevice;
		
    uint32 memoryTypeIndex = 0;
    VkMemoryRequirements memReqs = {};
    VkMemoryAllocateInfo memAlloc;
    ZeroVulkanStruct(memAlloc, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
		
    VkBufferCreateInfo bufferCreateInfo;
    ZeroVulkanStruct(bufferCreateInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
    bufferCreateInfo.usage = usageFlags;
    bufferCreateInfo.size  = size;
    vkCreateBuffer(vkDevice, &bufferCreateInfo, nullptr, &(dvkBuffer->Buffer));

    vkGetBufferMemoryRequirements(vkDevice, dvkBuffer->Buffer, &memReqs);
    device->GetMemoryManager().GetMemoryTypeFromProperties(memReqs.memoryTypeBits, memoryPropertyFlags, &memoryTypeIndex);
    memAlloc.allocationSize  = memReqs.size;
    memAlloc.memoryTypeIndex = memoryTypeIndex;
		
    vkAllocateMemory(vkDevice, &memAlloc, nullptr, &dvkBuffer->Memory);

    dvkBuffer->Size       = memAlloc.allocationSize;
    dvkBuffer->Alignment  = memReqs.alignment;
    dvkBuffer->UsageFlags = usageFlags;
    dvkBuffer->MemoryPropertyFlags = memoryPropertyFlags;

    if (data != nullptr)
    {
        dvkBuffer->Map();
        memcpy(dvkBuffer->Mapped, data, size);
        if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
        {
            dvkBuffer->Flush();
        }
        
        dvkBuffer->UnMap();
    }

    dvkBuffer->SetupDescriptor();
    dvkBuffer->Bind();

    return dvkBuffer;
}

VkResult VulkanBuffer::Map(VkDeviceSize size, VkDeviceSize offset)
{
    if(Mapped)
    {
        return VK_SUCCESS;
    }

    return vkMapMemory(Device,Memory,offset,Size,0,&Mapped);
}

void VulkanBuffer::UnMap()
{
    if(!Mapped)
    {
        return;
    }

    vkUnmapMemory(Device, Memory);
    Mapped = nullptr;
}

VkResult VulkanBuffer::Bind(VkDeviceSize offset)
{
    return vkBindBufferMemory(Device, Buffer, Memory, offset);
}

void VulkanBuffer::SetupDescriptor(VkDeviceSize size, VkDeviceSize offset)
{
    Descriptor.offset = offset;
    Descriptor.buffer = Buffer;
    Descriptor.range  = size;
}

void VulkanBuffer::CopyFrom(void* data, VkDeviceSize size)
{
    if (!Mapped)
    {
        RE_CORE_ERROR("data is not mapped");
        return;
    }
    
    memcpy(Mapped, data, size);
}

void VulkanBuffer::TransferBuffer(const Ref<VulkanDevice>& Device,const VulkanCommandPool& CommandPool,VulkanBuffer* SrcBuffer, VulkanBuffer* DstBuffer, VkDeviceSize size)
{
    auto cmdBuffer = VulkanCommandBuffer::Create(Device, CommandPool.m_CommandPool);
    cmdBuffer->Begin();
    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(cmdBuffer->CmdBuffer, SrcBuffer->Buffer, DstBuffer->Buffer, 1, &copyRegion);
    cmdBuffer->End();
    cmdBuffer->Submit();
}

void VulkanBuffer::TransferBuffer(const Ref<VulkanDevice>& Device, Ref<VulkanCommandBuffer> CommandBuffer,Ref<VulkanBuffer> SrcBuffer, Ref<VulkanBuffer> DstBuffer, VkDeviceSize size)
{
    CommandBuffer->Begin();
    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(CommandBuffer->CmdBuffer, SrcBuffer->Buffer, DstBuffer->Buffer, 1, &copyRegion);
    CommandBuffer->End();
    CommandBuffer->Submit();
}

VkResult VulkanBuffer::Flush(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = Memory;
    mappedRange.offset = offset;
    mappedRange.size   = size;
    return vkFlushMappedMemoryRanges(Device, 1, &mappedRange);
}

VkResult VulkanBuffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = Memory;
    mappedRange.offset = offset;
    mappedRange.size   = size;
    return vkInvalidateMappedMemoryRanges(Device, 1, &mappedRange);
}

void VulkanBuffer::TransferBufferImpl(VkDevice Device,VkCommandPool CommandPool,VkQueue TransferQueue,VkBuffer SrcBuffer, VkBuffer DstBuffer, VkDeviceSize size)
{
    //使用命令缓冲区执行内存传输的命令,从CommandPool中申请临时CommandList
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = CommandPool;
    allocInfo.commandBufferCount = 1;

    //创建录制Command
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(Device, &allocInfo, &commandBuffer);
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; //只使用一次

    //录制Transfer
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    VkBufferCopy copyRegion = {};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, SrcBuffer, DstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    //提交Transfer
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(TransferQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(TransferQueue); //等待传输队列变成Idle
        
    vkFreeCommandBuffers(Device, CommandPool, 1, &commandBuffer);
}