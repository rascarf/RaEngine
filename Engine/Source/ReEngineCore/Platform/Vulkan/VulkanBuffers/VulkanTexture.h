#pragma once
#include "Platform/Vulkan/VulkanCommonDefine.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanCommandBuffer.h"

VK_DEFINE_HANDLE( VmaAllocator )
VK_DEFINE_HANDLE( VmaAllocation )

class VulkanTexture
{
public:
    VulkanTexture(){}
    ~VulkanTexture();
    
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

    /*-------------------- Cube --------------------------*/
    static Ref<VulkanTexture> CreateCube(
        Ref<VulkanDevice> VulkanDevice,
        Ref<VulkanCommandBuffer> CmdBuffer,
        VkFormat Format,
        VkImageAspectFlags Aspect,
        int32 Width,
        int32 Height,
        bool Mipmap,
        VkImageUsageFlags Usage,
        VkSampleCountFlagBits SampleCount = VK_SAMPLE_COUNT_1_BIT,
        ImageLayoutBarrier ImageLayout = ImageLayoutBarrier::Undefined
    );

    static Ref<VulkanTexture> CreateCube(
        std::vector<std::string> FileNames,
        Ref<VulkanDevice> VulkanDevice,
        int32 Width,
        int32 Height,
        VkImageUsageFlags Usage,
        ImageLayoutBarrier imageLayout = ImageLayoutBarrier::PixelShaderRead
    );

    static Ref<VulkanTexture> CreateCubeRT(
        Ref<VulkanDevice> vulkanDevice,
        VkFormat format,
        VkImageAspectFlags aspect,
        int32 width,
        int32 height,
        VkImageUsageFlags usage,
        VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT
    );
    
    /*--------------------RT--------------------------*/
    
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
    static Ref<VulkanTexture> CreateFrameGraphAliasingTexture(
        Ref<VulkanDevice> vulkanDevice,
        Ref<VulkanTexture> AliasingTexture,
        VkFormat format,
        int32 width,
        int32 height,
        VkImageUsageFlags usage,
        int32 Mipmap,
        VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);

    static Ref<VulkanTexture> CreateFrameGraphTexture(
        Ref<VulkanDevice> vulkanDevice,
        VkFormat format,
        int32 width,
        int32 height,
        VkImageUsageFlags usage,
        int32 Mipmap,
        VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT);
public:
    bool bIsDepth();
    void UpdateSampler(
        VkFilter magFilter = VK_FILTER_LINEAR, 
        VkFilter minFilter = VK_FILTER_LINEAR,
        VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT, 
        VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT, 
        VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT
    );
public:
    //*************************** Util *************************************************//
    static void SetImageBarrierInfo(ImageLayoutBarrier source, ImageLayoutBarrier dest, VkImageMemoryBarrier& inOutBarrier, VkPipelineStageFlags& inOutSourceStage, VkPipelineStageFlags& inOutDestStage)
    {
        inOutSourceStage |= GetImageBarrierFlags(source, inOutBarrier.srcAccessMask, inOutBarrier.oldLayout);
        inOutDestStage   |= GetImageBarrierFlags(dest,   inOutBarrier.dstAccessMask, inOutBarrier.newLayout);
    }

    static void ImagePipelineBarrier(VkCommandBuffer CommandBuffer,Ref<VulkanTexture> Texture,ImageLayoutBarrier Source,ImageLayoutBarrier Dest,const VkImageSubresourceRange& subresourceRange,uint32_t SrcQueueFmily = VK_QUEUE_FAMILY_IGNORED,uint32_t DstQueueFamily = VK_QUEUE_FAMILY_IGNORED);
    static void ImagePipelineBarrier(VkCommandBuffer CommandBuffer,Ref<VulkanTexture> Texture,ImageLayoutBarrier Source,ImageLayoutBarrier Dest,uint32 BaseMiplevel,uint32 MipCount,bool IsDepth,uint32_t SrcQueueFmily = VK_QUEUE_FAMILY_IGNORED,uint32_t DstQueueFamily = VK_QUEUE_FAMILY_IGNORED);
    static void ImagePipelineBarrier(VkCommandBuffer CommandBuffer,VkImage Image,ImageLayoutBarrier Source,ImageLayoutBarrier Dest,const VkImageSubresourceRange& subresourceRange,uint32_t SrcQueueFmily = VK_QUEUE_FAMILY_IGNORED,uint32_t DstQueueFamily = VK_QUEUE_FAMILY_IGNORED);
    
public:
    Ref<VulkanDevice> Device = nullptr;
    
    VkImage Image = VK_NULL_HANDLE;
    VmaAllocation   mVmaAllocation;
    
    VkImageView ImageView = VK_NULL_HANDLE;
    VkImageLayout ImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    
    VkSampler ImageSampler = VK_NULL_HANDLE;
    VkDescriptorImageInfo DescriptorInfo;
    
    ImageLayoutBarrier mImageLayoutBarrier = ImageLayoutBarrier::Undefined;
    VkFormat Format = VK_FORMAT_R8G8B8A8_UNORM;
    VkSampleCountFlagBits NumSamples = VK_SAMPLE_COUNT_1_BIT;
    uint32_t OwnerQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    
    int32 Width = 0;
    int32 Height = 0;
    int32 Depth = 1;
    int32 MipLevels = 0;
    int32 LayerCount = 1;
    int32 BindlessIndex = -1;     // 标识此Texture在全局Texture中的序号
    
    bool bOnlyDepth = false; // 是否是DepthOnly的，区分DepthStencil
    bool IsUpload = false;
    bool IsCubeMap = false;
};
