#include "VulkanTexture.h"
#include "ImageLoader.h"
#include "assimp/code/OpenGEX/OpenGEXStructs.h"
#include "Math/Math.h"
#include "Platform/Vulkan/VulkanInstance.h"
#include "Resource/AssetManager/AssetManager.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanBuffer.h"

void VulkanTexture::UpdateSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode,VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV, VkSamplerAddressMode addressModeW)
{
}

Ref<VulkanTexture> VulkanTexture::Create2D(const std::string& filename, std::shared_ptr<VulkanDevice> vulkanDevice, Ref<VulkanCommandBuffer> cmdBuffer, VkImageUsageFlags imageUsageFlags, ImageLayoutBarrier imageLayout)
{
    uint32 DataSize = 0;
    uint8* DataPtr = nullptr;

    if(!AssetManager::ReadFile(filename,DataPtr,DataSize))
    {
        RE_CORE_ERROR("Failed to Load Image : {0}",filename.c_str());
    }

    int32 comp   = 0;
    int32 width  = 0;
    int32 height = 0;
    uint8* rgbaData = StbImage::LoadFromMemory(DataPtr, DataSize, &width, &height, &comp, 4);

    delete[] DataPtr;
    DataPtr = nullptr;

    if (rgbaData == nullptr)
    {
        RE_CORE_ERROR("Failed load image From StbImage: {0}", filename.c_str());
        return nullptr;
    }

    Ref<VulkanTexture> texture = Create2D(rgbaData, width * height * 4, VK_FORMAT_R8G8B8A8_UNORM, width, height, vulkanDevice, cmdBuffer, imageUsageFlags, imageLayout);

    StbImage::Free(rgbaData);

    return texture;
}

Ref<VulkanTexture> VulkanTexture::CreateDepthStencil(int32 width, int32 height,std::shared_ptr<VulkanDevice> vulkanDevice,PixelFormat DepthFormat, VkSampleCountFlagBits NumSamples,VkImageUsageFlags imageUsageFlags,ImageLayoutBarrier imageLayout)
{
    int32 fwidth    = width;
    int32 fheight   = height;
    VkDevice device = vulkanDevice->GetInstanceHandle();

    Ref<VulkanTexture> texture   = CreateRef<VulkanTexture>();
    texture->Device = device;
    texture->Height = height;
    texture->Width = width;
    texture->Format = PixelFormatToVkFormat(DepthFormat, false);
    texture->NumSamples = NumSamples;

    VkImageCreateInfo imageCreateInfo;
    ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
    imageCreateInfo.imageType   = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format      = PixelFormatToVkFormat(DepthFormat, false);
    imageCreateInfo.extent      = { (uint32)fwidth, (uint32)fheight, 1 };
    imageCreateInfo.mipLevels   = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.samples     = NumSamples;
    imageCreateInfo.tiling      = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage       = imageUsageFlags;
    imageCreateInfo.flags       = 0;
    imageCreateInfo.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    VERIFYVULKANRESULT(vkCreateImage(device, &imageCreateInfo, VULKAN_CPU_ALLOCATOR, &texture->Image))

    VkMemoryRequirements memRequire;
    vkGetImageMemoryRequirements(device, texture->Image, &memRequire);
    uint32 memoryTypeIndex = 0;
    VERIFYVULKANRESULT(vulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(memRequire.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryTypeIndex))

    VkMemoryAllocateInfo memAllocateInfo;
    ZeroVulkanStruct(memAllocateInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
    memAllocateInfo.allocationSize  = memRequire.size;
    memAllocateInfo.memoryTypeIndex = memoryTypeIndex;
    vkAllocateMemory(device, &memAllocateInfo, VULKAN_CPU_ALLOCATOR, &texture->ImageMemory);
    vkBindImageMemory(device, texture->Image, texture->ImageMemory, 0);

    VkImageViewCreateInfo imageViewCreateInfo;
    ZeroVulkanStruct(imageViewCreateInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format   = PixelFormatToVkFormat(DepthFormat, false);
    imageViewCreateInfo.flags    = 0;
    imageViewCreateInfo.image    = texture->Image;
    imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
    imageViewCreateInfo.subresourceRange.levelCount     = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount     = 1;
    VERIFYVULKANRESULT(vkCreateImageView(device, &imageViewCreateInfo, VULKAN_CPU_ALLOCATOR, &texture->ImageView))

    return texture;
}

Ref<VulkanTexture> VulkanTexture::Create2D(const uint8* rgbaData,uint32 size,VkFormat format,int32 width,int32 height,std::shared_ptr<VulkanDevice> vulkanDevice,Ref<VulkanCommandBuffer> cmdBuffer,VkImageUsageFlags imageUsageFlags,ImageLayoutBarrier imageLayout)
{
    int32 MipLevels = Math::FloorToInt(Math::Log2(Math::Max(width,height))) + 1;
    VkDevice device = vulkanDevice->GetInstanceHandle();
    
    auto StagingBuffer =  VulkanBuffer::CreateBuffer(
        vulkanDevice,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        , size);

    StagingBuffer->Map();
    StagingBuffer->CopyFrom((void*)rgbaData,size);
    StagingBuffer->UnMap();

    uint32 MemoryTypeIndex = 0;
    VkMemoryRequirements MemReqs = {};
    VkMemoryAllocateInfo memAllocInfo;
    ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

    // image info
    VkImage                         image = VK_NULL_HANDLE;
    VkDeviceMemory                  imageMemory = VK_NULL_HANDLE;
    VkImageView                     imageView = VK_NULL_HANDLE;
    VkSampler                       imageSampler = VK_NULL_HANDLE;
    VkDescriptorImageInfo           descriptorInfo = {};

    if (!(imageUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
    {
        imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if (!(imageUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT))
    {
        imageUsageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    VkImageCreateInfo ImageCreateInfo;
    ZeroVulkanStruct(ImageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
    ImageCreateInfo.imageType       = VK_IMAGE_TYPE_2D;
    ImageCreateInfo.format          = format;
    ImageCreateInfo.mipLevels       = MipLevels;
    ImageCreateInfo.arrayLayers     = 1;
    ImageCreateInfo.samples         = VK_SAMPLE_COUNT_1_BIT;
    ImageCreateInfo.tiling          = VK_IMAGE_TILING_OPTIMAL;
    ImageCreateInfo.sharingMode     = VK_SHARING_MODE_EXCLUSIVE;
    ImageCreateInfo.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageCreateInfo.extent          = { (uint32_t)width, (uint32_t)height, 1 };
    ImageCreateInfo.usage           = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VERIFYVULKANRESULT(vkCreateImage(device, &ImageCreateInfo, VULKAN_CPU_ALLOCATOR, &image));
    
    // bind image Memory
    vkGetImageMemoryRequirements(device, image, &MemReqs);
    vulkanDevice->GetMemoryManager().GetMemoryTypeFromProperties(MemReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &MemoryTypeIndex);
    memAllocInfo.allocationSize  = MemReqs.size;
    memAllocInfo.memoryTypeIndex = MemoryTypeIndex;
    VERIFYVULKANRESULT(vkAllocateMemory(device, &memAllocInfo, VULKAN_CPU_ALLOCATOR, &imageMemory));
    VERIFYVULKANRESULT(vkBindImageMemory(device, image, imageMemory, 0));

    // start record
    cmdBuffer->Begin();

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.levelCount     = 1;
    subresourceRange.layerCount     = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.baseMipLevel   = 0;

    // undefined to TransferDest
    ImagePipelineBarrier(cmdBuffer->CmdBuffer, image, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::TransferDest, subresourceRange);
    VkBufferImageCopy bufferCopyRegion = {};
    bufferCopyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopyRegion.imageSubresource.mipLevel       = 0;
    bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
    bufferCopyRegion.imageSubresource.layerCount     = 1;
    bufferCopyRegion.imageExtent.width  = width;
    bufferCopyRegion.imageExtent.height = height;
    bufferCopyRegion.imageExtent.depth  = 1;
    
    vkCmdCopyBufferToImage(cmdBuffer->CmdBuffer, StagingBuffer->Buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);
    ImagePipelineBarrier(cmdBuffer->CmdBuffer,image,ImageLayoutBarrier::TransferDest, ImageLayoutBarrier::TransferSource,subresourceRange);
    
    for(uint32_t i = 1 ; i <(uint32_t) MipLevels ; i++)
    {
        VkImageBlit imageBlit = {};

        int32 mip0Width  = Math::Max(width >> (i - 1), 1);
        int32 mip0Height = Math::Max(height >> (i - 1), 1);
        int32 mip1Width  = Math::Max(width >> (i - 0), 1);
        int32 mip1Height = Math::Max(height >> (i - 0), 1);

        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.layerCount = 1;
        imageBlit.srcSubresource.mipLevel   = i - 1;
        imageBlit.srcOffsets[1].x = int32_t(mip0Width);
        imageBlit.srcOffsets[1].y = int32_t(mip0Height);
        imageBlit.srcOffsets[1].z = 1;

        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.layerCount = 1;
        imageBlit.dstSubresource.mipLevel   = i;
        imageBlit.dstOffsets[1].x = int32_t(mip1Width);
        imageBlit.dstOffsets[1].y = int32_t(mip1Height);
        imageBlit.dstOffsets[1].z = 1;

        VkImageSubresourceRange mipSubRange = {};
        mipSubRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        mipSubRange.baseMipLevel   = i;
        mipSubRange.levelCount     = 1;
        mipSubRange.layerCount     = 1;
        mipSubRange.baseArrayLayer = 0;

        // undefined to dst
        ImagePipelineBarrier(cmdBuffer->CmdBuffer, image, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::TransferDest, mipSubRange);

        // blit image
        vkCmdBlitImage(cmdBuffer->CmdBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

        // dst to src
        ImagePipelineBarrier(cmdBuffer->CmdBuffer, image, ImageLayoutBarrier::TransferDest, ImageLayoutBarrier::TransferSource, mipSubRange);
    }

    subresourceRange.levelCount = MipLevels;
    ImagePipelineBarrier(cmdBuffer->CmdBuffer, image, ImageLayoutBarrier::TransferSource, imageLayout, subresourceRange);

    cmdBuffer->End();
    cmdBuffer->Submit();

    VkSamplerCreateInfo samplerInfo;
    ZeroVulkanStruct(samplerInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
    samplerInfo.magFilter        = VK_FILTER_LINEAR;
    samplerInfo.minFilter        = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode       = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.compareOp        = VK_COMPARE_OP_NEVER;
    samplerInfo.borderColor      = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.maxAnisotropy    = 1.0;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxLod           = (float)MipLevels;
    samplerInfo.minLod           = 0.0f;
    VERIFYVULKANRESULT(vkCreateSampler(device, &samplerInfo, VULKAN_CPU_ALLOCATOR, &imageSampler));

    VkImageViewCreateInfo viewInfo;
    ZeroVulkanStruct(viewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
    viewInfo.image      = image;
    viewInfo.viewType   = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format     = format;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.levelCount = MipLevels;
    VERIFYVULKANRESULT(vkCreateImageView(device, &viewInfo, VULKAN_CPU_ALLOCATOR, &imageView));

    descriptorInfo.sampler     = imageSampler;
    descriptorInfo.imageView   = imageView;
    descriptorInfo.imageLayout = GetImageLayout(imageLayout);

    Ref<VulkanTexture> texture   = CreateRef<VulkanTexture>();
    texture->DescriptorInfo = descriptorInfo;
    texture->Format         = format;
    texture->Height         = height;
    texture->Image          = image;
    texture->ImageLayout    = GetImageLayout(imageLayout);
    texture->ImageMemory    = imageMemory;
    texture->ImageSampler   = imageSampler;
    texture->ImageView      = imageView;
    texture->Device         = device;
    texture->Width          = width;
    texture->MipLevels      = MipLevels;
    texture->LayerCount     = 1;

    return texture;
}

// Ref<VulkanTexture> VulkanTexture::Create2DArray(const std::vector<std::string> filenames,std::shared_ptr<VulkanDevice> vulkanDevice, Ref<VulkanCommandBuffer> cmdBuffer)
// {
// }

// Ref<VulkanTexture> VulkanTexture::Create3D(VkFormat format, const uint8* rgbaData, int32 size, int32 width,
//     int32 height, int32 depth, std::shared_ptr<VulkanDevice> vulkanDevice, Ref<VulkanCommandBuffer> cmdBuffer)
// {
// }
