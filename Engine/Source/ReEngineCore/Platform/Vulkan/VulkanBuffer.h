#pragma once
#include "VulkanCommonDefine.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"

class VulkanBuffer
{
public:
    ~VulkanBuffer()
    {
         if(Buffer != VK_NULL_HANDLE)
         {
             vkDestroyBuffer(Device, Buffer, nullptr);
             Buffer = VK_NULL_HANDLE;
         }
        
         if(Memory != VK_NULL_HANDLE)
         {
             vkFreeMemory(Device, Memory, nullptr);
             Memory = VK_NULL_HANDLE;
         }
    }
public:
    VkDevice				Device = VK_NULL_HANDLE;
    
    VkBuffer				Buffer = VK_NULL_HANDLE;
    VkDeviceMemory			Memory = VK_NULL_HANDLE;

    VkDescriptorBufferInfo	Descriptor;

    VkDeviceSize			Size = 0;
    VkDeviceSize			Alignment = 0;

    void*					Mapped = nullptr;

    VkBufferUsageFlags		UsageFlags;
    VkMemoryPropertyFlags	MemoryPropertyFlags;

public:
    //创建Buffer
    static VulkanBuffer* CreateBuffer(std::shared_ptr<VulkanDevice> device, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data = nullptr);
    static void TransferBuffer(const VulkanInstance& Instance,const VulkanCommandPool& CommandPool,VulkanBuffer* SrcBuffer,VulkanBuffer* DstBuffer,VkDeviceSize size);

    //开始数据映射
    VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    //结束数据映射
    void UnMap();

    //绑定当前Buffer
    VkResult Bind(VkDeviceSize offset = 0);

    //设置描述符
    void SetupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    //拷贝数据到映射的区间
    void CopyFrom(void* data, VkDeviceSize size);
    
    VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

private:
    VulkanBuffer(){}
    static void TransferBufferImpl(VkDevice Device,VkCommandPool CommandPool,VkQueue TransferQueue,VkBuffer SrcBuffer, VkBuffer DstBuffer, VkDeviceSize size);
};
