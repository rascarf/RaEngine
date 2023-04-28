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

class VulkanTexture
{
public:
    VulkanTexture(){}


    ~VulkanTexture()
    {
        if (ImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(Device, ImageView, VULKAN_CPU_ALLOCATOR);
            ImageView = VK_NULL_HANDLE;
        }
        
        if (Image != VK_NULL_HANDLE)
        {
            vkDestroyImage(Device, Image, VULKAN_CPU_ALLOCATOR);
            Image = VK_NULL_HANDLE;
        }
        
        if (ImageSampler != VK_NULL_HANDLE) {
            vkDestroySampler(Device, ImageSampler, VULKAN_CPU_ALLOCATOR);
            ImageSampler = VK_NULL_HANDLE;
        }
        
        if (ImageMemory != VK_NULL_HANDLE) {
            vkFreeMemory(Device, ImageMemory, VULKAN_CPU_ALLOCATOR);
            ImageMemory = VK_NULL_HANDLE;
        }
    }

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

    static  Ref<VulkanTexture> Create2D(
        const std::string& filename,
        std::shared_ptr<VulkanDevice> vulkanDevice,
        Ref<VulkanCommandBuffer> cmdBuffer,
        VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        ImageLayoutBarrier imageLayout = ImageLayoutBarrier::PixelShaderRead
    );

    static Ref<VulkanTexture> CreateDepthStencil(
            int32 width,
            int32 height,
            std::shared_ptr<VulkanDevice> vulkanDevice,
            PixelFormat              DepthFormat,
            VkSampleCountFlagBits NumSamples = VK_SAMPLE_COUNT_1_BIT,
            VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            ImageLayoutBarrier imageLayout = ImageLayoutBarrier::DepthStencilAttachment
        );

    // static  Ref<VulkanTexture> Create2D(
    // std::shared_ptr<VulkanDevice> vulkanDevice,
    // Ref<VulkanCommandBuffer> cmdBuffer,
    // VkFormat format,
    // VkImageAspectFlags aspect,
    // int32 width,
    // int32 height,
    // VkImageUsageFlags usage,
    // VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT,
    // ImageLayoutBarrier imageLayout = ImageLayoutBarrier::Undefined
    // );
    /*----------------------------------------------*/


public:

    VkDevice Device = nullptr;
    VkImage Image = VK_NULL_HANDLE;
    VkDeviceMemory ImageMemory = VK_NULL_HANDLE;
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
};
