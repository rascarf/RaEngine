#pragma once
#include "Platform/Vulkan/VulkanCommonDefine.h"
#include "Platform/Vulkan/VulkanDevice.h"


class NewVulkanCommandBuffer
{
public:
    ~NewVulkanCommandBuffer();

public:
    virtual void Begin();
    virtual void End();

    virtual void Submit(VkSemaphore* SignalSemaphore = nullptr);

    //分配使用单次的
    static Ref<NewVulkanCommandBuffer> Create(std::shared_ptr<VulkanDevice> vulkanDevice, VkCommandPool commandPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,Ref<VulkanQueue> queue = nullptr);
public:
    VkCommandBuffer						CmdBuffer = VK_NULL_HANDLE;
    VkCommandPool						CommandPool = VK_NULL_HANDLE;
    std::weak_ptr<VulkanDevice>		    m_VulkanDevice;

    Ref<VulkanQueue>                    queue = nullptr;

    bool								IsBegun;

public:
    void  UploadTextureData(Ref<class VulkanTexture> DstTexture,void* SrcData,Ref<class VulkanBuffer> StagingBuffer,uint32 StagingBuffer_Offset);
    void  UploadCubeTextureData(Ref<class VulkanTexture> DstTexture,Ref<class VulkanBuffer> StagingBuffer,uint32 SliceSize);
    void  GenerateMipMaps(Ref<class VulkanTexture> DstTexture);
    void  CopyImage(Ref<VulkanTexture> Src,Ref<VulkanTexture> Dst);

    
public: // 封装好的Barrier
    void  ReadBackBarrier(Ref<class VulkanBuffer> Dst); // buffer写完给回读的Barrier
    void  BufferWriteBarrier(Ref<class VulkanBuffer> Dst);// buffer写完给下一个Buffer读的Barrier
   
};

class VulkanCommandBuffer : public NewVulkanCommandBuffer
{
public:
    ~VulkanCommandBuffer();

public:
    virtual void Begin() override;
    virtual void End() override;
    virtual void Submit(VkSemaphore* SignalSemaphore = nullptr) override;

    //分配使用单次的
    static Ref<VulkanCommandBuffer> Create(std::shared_ptr<VulkanDevice> vulkanDevice, VkCommandPool commandPool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,std::shared_ptr<VulkanQueue> queue = nullptr);
public:

    VkFence								Fence = VK_NULL_HANDLE;
    std::vector<VkPipelineStageFlags>	WaitFlags;
    std::vector<VkSemaphore>			WaitSemaphores;
};

