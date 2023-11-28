#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "vk_mem_alloc.h"

VulkanBuffer::~VulkanBuffer()
{
    UnMap();
    
    if(Buffer != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(Device->vma_allocator,Buffer,VmaAllocation);
    }

    Device.reset();
}

Ref<VulkanBuffer> VulkanBuffer::CreateBuffer(std::shared_ptr<VulkanDevice> device, VkBufferUsageFlags usageFlags,
                                             VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void* data)
{
    Ref<VulkanBuffer> dvkBuffer = CreateRef<VulkanBuffer>();
    dvkBuffer->Device = device;
		
    VkBufferCreateInfo bufferCreateInfo;
    ZeroVulkanStruct(bufferCreateInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
    bufferCreateInfo.usage = usageFlags;
    bufferCreateInfo.size  = size;

    VmaAllocationCreateInfo MemoryInfo{};
    MemoryInfo.flags = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    MemoryInfo.usage = VMA_MEMORY_USAGE_AUTO;

    VmaAllocationInfo AllocationInfo;
    vmaCreateBuffer(device->vma_allocator,&bufferCreateInfo,&MemoryInfo,
        &dvkBuffer->Buffer,&dvkBuffer->VmaAllocation,&AllocationInfo);
    
    dvkBuffer->Size       = AllocationInfo.size;
    dvkBuffer->UsageFlags = usageFlags;
    dvkBuffer->MemoryPropertyFlags = memoryPropertyFlags;

    if (data != nullptr)
    {
        dvkBuffer->Map();
        memcpy(dvkBuffer->Mapped, data, size);
        dvkBuffer->UnMap();
    }

    dvkBuffer->SetupDescriptor();

    return dvkBuffer;
}

VkResult VulkanBuffer::Map(VkDeviceSize size, VkDeviceSize offset)
{
    if(Mapped)
    {
        return VK_SUCCESS;
    }

    return vmaMapMemory(Device->vma_allocator,VmaAllocation,&Mapped);
}

void VulkanBuffer::UnMap()
{
    if(!Mapped)
    {
        return;
    }

    vmaUnmapMemory(Device->vma_allocator, VmaAllocation);
    Mapped = nullptr;
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