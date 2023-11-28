#pragma once
#include "Platform/Vulkan/VulkanCommonDefine.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanCommandBuffer.h"

FORCE_INLINE void SetImageBarrierInfo(ImageLayoutBarrier source, ImageLayoutBarrier dest, VkImageMemoryBarrier& inOutBarrier, VkPipelineStageFlags& inOutSourceStage, VkPipelineStageFlags& inOutDestStage)
{
    inOutSourceStage |= GetImageBarrierFlags(source, inOutBarrier.srcAccessMask, inOutBarrier.oldLayout);
    inOutDestStage   |= GetImageBarrierFlags(dest,   inOutBarrier.dstAccessMask, inOutBarrier.newLayout);
}

FORCE_INLINE void ImagePipelineBarrier(VkCommandBuffer CommandBuffer,VkImage Image,ImageLayoutBarrier Source,ImageLayoutBarrier Dest,const VkImageSubresourceRange& subresourceRange)
{
    VkImageMemoryBarrier ImageBarrier;
    ZeroVulkanStruct(ImageBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
    ImageBarrier.image = Image;
    ImageBarrier.subresourceRange    = subresourceRange;
    ImageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ImageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    VkPipelineStageFlags sourceStages = (VkPipelineStageFlags)0;
    VkPipelineStageFlags destStages   = (VkPipelineStageFlags)0;
    
    SetImageBarrierInfo(Source, Dest, ImageBarrier, sourceStages, destStages);

    if (Source == ImageLayoutBarrier::Present)
    {
        sourceStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    else if (Dest == ImageLayoutBarrier::Present)
    {
        destStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }

    vkCmdPipelineBarrier(CommandBuffer, sourceStages, destStages, 0, 0, nullptr, 0, nullptr, 1, &ImageBarrier); 
}

VK_DEFINE_HANDLE( VmaAllocator )
VK_DEFINE_HANDLE( VmaAllocation )

class VulkanTexture
{
public:
    VulkanTexture(){}


    ~VulkanTexture();


    void UpdateSampler(
        VkFilter magFilter = VK_FILTER_LINEAR, 
        VkFilter minFilter = VK_FILTER_LINEAR,
        VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT, 
        VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT, 
        VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT
        );
    
    // static Ref<VulkanTexture> Create2DArray(const std::vector<std::string> filenames, std::shared_ptr<VulkanDevice> vulkanDevice, Ref<VulkanCommandBuffer> cmdBuffer);
    // static Ref<VulkanTexture> Create3D(VkFormat format, const uint8* rgbaData, int32 size, int32 width, int32 height, int32 depth, std::shared_ptr<VulkanDevice> vulkanDevice, Ref<VulkanCommandBuffer> cmdBuffer);

    /*------------------ 2D ---------------------------*/

    /*读取一张贴图*/
    //创建的时候带有数据的
    static Ref<VulkanTexture> Create2D(
    const uint8* rgbaData,
    uint32 size,
    VkFormat format,
    int32 width,
    int32 height,
    std::shared_ptr<VulkanDevice> vulkanDevice,
    Ref<VulkanCommandBuffer> cmdBuffer,
    VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    ImageLayoutBarrier imageLayout = ImageLayoutBarrier::PixelShaderRead
);

    /*读取一张贴图*/
    //创建的时候带有数据的
    static  Ref<VulkanTexture> Create2D(
        const std::string& filename,
        std::shared_ptr<VulkanDevice> vulkanDevice,
        Ref<VulkanCommandBuffer> cmdBuffer,
        VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        ImageLayoutBarrier imageLayout = ImageLayoutBarrier::PixelShaderRead
    );
    
    /*创建StorageBuffer*/
    //创建的时候没有数据
    static  Ref<VulkanTexture> Create2D(
    std::shared_ptr<VulkanDevice> vulkanDevice,
    Ref<VulkanCommandBuffer> cmdBuffer,
    VkFormat format,
    VkImageAspectFlags aspect,
    int32 width,
    int32 height,
    VkImageUsageFlags usage,
    VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
    ImageLayoutBarrier imageLayout = ImageLayoutBarrier::Undefined
    );

    /*----------------------------------------------*/
    
    /*创建DepthStencil*/
    //没有初始数据的
    static Ref<VulkanTexture> CreateDepthStencil(
        int32 width,
        int32 height,
        Ref<VulkanDevice> vulkanDevice,
        PixelFormat              DepthFormat,
        VkSampleCountFlagBits NumSamples = VK_SAMPLE_COUNT_1_BIT,
        VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        ImageLayoutBarrier imageLayout = ImageLayoutBarrier::DepthStencilAttachment
    );
    
    /*创建RT*/
    //没有初始数据的
    static Ref<VulkanTexture> CreateRenderTarget(
        Ref<VulkanDevice> vulkanDevice,
        VkFormat format,
        VkImageAspectFlags aspect,
        int32 width,
        int32 height,
        VkImageUsageFlags usage,
        VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT
    );

    //创建Attachment
    //没有初始数据的
    static Ref<VulkanTexture> CreateAttachment(
        Ref<VulkanDevice> vulkanDevice,
        VkFormat Format,
        VkImageAspectFlags Aspect,
        int32 Width,
        int32 Height,
        VkImageUsageFlags Usage
    );

    //------------Aliasing----------------//
    // 如果返回的指针是空的，那么说明不兼容
    static Ref<VulkanTexture> CreateAliasingTexture(
        Ref<VulkanDevice> vulkanDevice,
        Ref<VulkanTexture> AliasingTexture,
        VkFormat format,
        VkImageAspectFlags aspect,
        int32 width,
        int32 height,
        VkImageUsageFlags usage,
        VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);
    
public:
    Ref<VulkanDevice> Device = nullptr;
    
    VkImage Image = VK_NULL_HANDLE;
    VmaAllocation   mVmaAllocation;
    
    VkImageView ImageView = VK_NULL_HANDLE;
    VkImageLayout ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkSampler ImageSampler = VK_NULL_HANDLE;
    VkDescriptorImageInfo DescriptorInfo;

    int32 Width = 0;
    int32 Height = 0;
    int32 Depth = 1;
    int32 MipLevels = 0;

    bool IsCubeMap = false;
    int32 LayerCount = 1;
    VkFormat Format = VK_FORMAT_R8G8B8A8_UNORM;
    VkSampleCountFlagBits NumSamples = VK_SAMPLE_COUNT_1_BIT;

    // 标识此Texture在全局Texture中的序号
    int32 BindlessIndex;
};
