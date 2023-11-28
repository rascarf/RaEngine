#pragma once
#include "VulkanCommandBuffer.h"
#include "Platform/Vulkan/VulkanCommandPool.h"

VK_DEFINE_HANDLE( VmaAllocator )
VK_DEFINE_HANDLE( VmaAllocation )

class VulkanBuffer
{
public:
    ~VulkanBuffer();
public:
    std::shared_ptr<VulkanDevice>	Device = VK_NULL_HANDLE;
    
    VkBuffer				Buffer = VK_NULL_HANDLE;
    VmaAllocation           VmaAllocation;
    VkDeviceMemory          DeviceMemory;

    VkDescriptorBufferInfo	Descriptor;

    VkDeviceSize			Size = 0;

    void*					Mapped = nullptr;

    VkBufferUsageFlags		UsageFlags;
    VkMemoryPropertyFlags	MemoryPropertyFlags;

public:
    //创建Buffer
    static Ref<VulkanBuffer> CreateBuffer(std::shared_ptr<VulkanDevice> device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data = nullptr);
    static void TransferBuffer(const Ref<VulkanDevice>& Device,const VulkanCommandPool& CommandPool,VulkanBuffer* SrcBuffer, VulkanBuffer* DstBuffer, VkDeviceSize size);
    static void TransferBuffer(const Ref<VulkanDevice>& Device, Ref<VulkanCommandBuffer> CommandBuffer,Ref<VulkanBuffer> SrcBuffer, Ref<VulkanBuffer> DstBuffer, VkDeviceSize size);

    //开始数据映射
    VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    //结束数据映射
    void UnMap();

    //设置描述符
    void SetupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    //拷贝数据到映射的区间
    void CopyFrom(void* data, VkDeviceSize size);

    VkDeviceAddress GetDeviceAddress() const 
    {
        VkBufferDeviceAddressInfo info = {VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
        info.buffer = Buffer;
        return vkGetBufferDeviceAddress(Device->GetInstanceHandle(), &info);
    }

private:
    static void TransferBufferImpl(VkDevice Device,VkCommandPool CommandPool,VkQueue TransferQueue,VkBuffer SrcBuffer, VkBuffer DstBuffer, VkDeviceSize size);
};
