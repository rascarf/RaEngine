#include "VulkanTexture.h"
#include "ImageLoader.h"
#include "Math/Math.h"
#include "Platform/Vulkan/VulkanInstance.h"
#include "Resource/AssetManager/AssetManager.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanBuffer.h"

#include "vk_mem_alloc.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Renderer/RHI/Texture.h"

VulkanTexture::~VulkanTexture()
{
    if (ImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(Device->GetInstanceHandle(), ImageView, VULKAN_CPU_ALLOCATOR);
        ImageView = VK_NULL_HANDLE;
    }
    
    if (ImageSampler != VK_NULL_HANDLE) {
        vkDestroySampler(Device->GetInstanceHandle(), ImageSampler, VULKAN_CPU_ALLOCATOR);
        ImageSampler = VK_NULL_HANDLE;
    }
    
    if (Image != VK_NULL_HANDLE)
    {
        vmaDestroyImage(Device->vma_allocator,Image,mVmaAllocation);
    }

    Device.reset();
}


void VulkanTexture::UpdateSampler(VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode,VkSamplerAddressMode addressModeU, VkSamplerAddressMode addressModeV, VkSamplerAddressMode addressModeW)
{
    VkSamplerCreateInfo samplerInfo;
    ZeroVulkanStruct(samplerInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
    samplerInfo.magFilter        = magFilter;
    samplerInfo.minFilter        = minFilter;
    samplerInfo.mipmapMode       = mipmapMode;
    samplerInfo.addressModeU     = addressModeU;
    samplerInfo.addressModeV     = addressModeV;
    samplerInfo.addressModeW     = addressModeW;
    samplerInfo.compareOp        = VK_COMPARE_OP_NEVER;
    samplerInfo.borderColor      = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.maxAnisotropy    = 1.0;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxLod           = 1.0f;
    VERIFYVULKANRESULT(vkCreateSampler(Device->GetInstanceHandle(), &samplerInfo, VULKAN_CPU_ALLOCATOR, &ImageSampler));

    if (DescriptorInfo.sampler)
    {
        vkDestroySampler(Device->GetInstanceHandle(), DescriptorInfo.sampler, VULKAN_CPU_ALLOCATOR);
    }
    
    DescriptorInfo.sampler = ImageSampler;
}



Ref<VulkanTexture> VulkanTexture::Create2D(const std::string& filename, std::shared_ptr<VulkanDevice> vulkanDevice, Ref<VulkanCommandBuffer> cmdBuffer, VkImageUsageFlags imageUsageFlags, ImageLayoutBarrier imageLayout,VkFormat LoadFormat)
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
    texture->Device = vulkanDevice;
    texture->Height = height;
    texture->Width = width;
    texture->Format = PixelFormatToVkFormat(DepthFormat, false);
    texture->NumSamples = NumSamples;
    texture->IsUpload       = true;

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

    VmaAllocationCreateInfo MemoryInfo{};
    MemoryInfo.usage = VMA_MEMORY_USAGE_AUTO;
    VERIFYVULKANRESULT(vmaCreateImage(vulkanDevice->vma_allocator,&imageCreateInfo,&MemoryInfo,&texture->Image,&texture->mVmaAllocation,nullptr));
    
    VkImageViewCreateInfo imageViewCreateInfo;
    ZeroVulkanStruct(imageViewCreateInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format   = PixelFormatToVkFormat(DepthFormat, false);
    imageViewCreateInfo.flags    = 0;
    imageViewCreateInfo.image    = texture->Image;
    imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    if(texture->bOnlyDepth)
    {
        imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    
    imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
    imageViewCreateInfo.subresourceRange.levelCount     = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount     = 1;
    
    VERIFYVULKANRESULT(vkCreateImageView(device, &imageViewCreateInfo, VULKAN_CPU_ALLOCATOR, &texture->ImageView))

    return texture;
}

Ref<VulkanTexture> VulkanTexture::Create2D(Ref<VulkanDevice> vulkanDevice,Ref<VulkanCommandBuffer> cmdBuffer, VkFormat format, VkImageAspectFlags aspect, int32 width, int32 height,VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount, ImageLayoutBarrier imageLayout)
{
    VkDevice device = vulkanDevice->GetInstanceHandle();

    int32 mipLevels = 1;

    // image info
    VkImage                         image = VK_NULL_HANDLE;
    VmaAllocation                   imageAllocation;
    VkImageView                     imageView = VK_NULL_HANDLE;
    VkSampler                       imageSampler = VK_NULL_HANDLE;
    VkDescriptorImageInfo           descriptorInfo = {};

    // 创建image
    VkImageCreateInfo imageCreateInfo;
    ZeroVulkanStruct(imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
    imageCreateInfo.imageType       = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format          = format;
    imageCreateInfo.mipLevels       = mipLevels;
    imageCreateInfo.arrayLayers     = 1;
    imageCreateInfo.samples         = sampleCount;
    imageCreateInfo.tiling          = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.sharingMode     = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.extent          = { (uint32_t)width, (uint32_t)height, (uint32_t)1 };
    imageCreateInfo.usage           = usage;

    // bind image buffer
    VmaAllocationCreateInfo MemoryInfo{};
    MemoryInfo.usage = VMA_MEMORY_USAGE_AUTO;
    VERIFYVULKANRESULT(vmaCreateImage(vulkanDevice->vma_allocator,&imageCreateInfo,&MemoryInfo,&image,&imageAllocation,nullptr));

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
    samplerInfo.maxLod           = (float)mipLevels;
    samplerInfo.minLod           = 0.0f;
    VERIFYVULKANRESULT(vkCreateSampler(device, &samplerInfo, VULKAN_CPU_ALLOCATOR, &imageSampler));

    VkImageViewCreateInfo viewInfo;
    ZeroVulkanStruct(viewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
    viewInfo.image      = image;
    viewInfo.viewType   = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format     = format;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    viewInfo.subresourceRange.aspectMask     = aspect;
    viewInfo.subresourceRange.layerCount     = 1;
    viewInfo.subresourceRange.levelCount     = mipLevels;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    VERIFYVULKANRESULT(vkCreateImageView(device, &viewInfo, VULKAN_CPU_ALLOCATOR, &imageView));

    if (cmdBuffer != nullptr && imageLayout != ImageLayoutBarrier::Undefined)
    {
        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.levelCount     = mipLevels;
        subresourceRange.layerCount     = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.baseMipLevel   = 0;
        
        ImagePipelineBarrier(cmdBuffer->CmdBuffer, image, ImageLayoutBarrier::Undefined, imageLayout, subresourceRange);
    }
    else
    {
        imageLayout = ImageLayoutBarrier::Undefined;
    }

    descriptorInfo.sampler     = imageSampler;
    descriptorInfo.imageView   = imageView;
    descriptorInfo.imageLayout = GetImageLayout(imageLayout);

    Ref<VulkanTexture> Texture = CreateRef<VulkanTexture>();
    Texture->DescriptorInfo = descriptorInfo;
    Texture->Format         = format;
    Texture->Width          = width;
    Texture->Height         = height;
    Texture->Depth          = 1;
    Texture->Image          = image;
    Texture->ImageLayout    = GetImageLayout(imageLayout);
    Texture->mVmaAllocation = imageAllocation;
    Texture->ImageSampler   = imageSampler;
    Texture->ImageView      = imageView;
    Texture->Device         = vulkanDevice;
    Texture->MipLevels      = mipLevels;
    Texture->LayerCount     = 1;
    Texture->NumSamples     = sampleCount;
    Texture->IsUpload       = false;

    return Texture;
}

Ref<VulkanTexture> VulkanTexture::Create2DWithData(const std::string& filename,std::shared_ptr<VulkanDevice> vulkanDevice, Ref<VulkanCommandBuffer> cmdBuffer, VkFormat LoadFormat,VkImageAspectFlags  Aspect,VkImageUsageFlags imageUsageFlags, ImageLayoutBarrier imageLayout, VkSampleCountFlagBits sampleCount)
{
    if(!cmdBuffer)
    {
        cmdBuffer = VulkanCommandBuffer::Create(
           vulkanDevice,
           vulkanDevice->mContext->CommandPool->m_ComputeCommandPool,
           VK_COMMAND_BUFFER_LEVEL_PRIMARY,
           vulkanDevice->GetTransferQueue()
        );
    }
    
    cmdBuffer->Begin();
    
    const Ref<TextureData> Data = TextureData::FromeFile(filename);
    
    Ref<VulkanTexture> ResultImage =  Create2D(
        vulkanDevice,
        cmdBuffer,
        LoadFormat,
        Aspect,
        Data->width(),
        Data->height(),
        imageUsageFlags,
        sampleCount,
        imageLayout
    );

    Ref<VulkanBuffer> StagingBuffer = VulkanBuffer::CreateBuffer(vulkanDevice,VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,Data->pitch() * Data->height(),Data->pixels<void>());

    cmdBuffer->UploadTextureData(ResultImage,nullptr,StagingBuffer,0);

    cmdBuffer->End();
    cmdBuffer->Submit();

    StagingBuffer.reset();
    return ResultImage;
}

Ref<VulkanTexture> VulkanTexture::CreateRenderTarget(std::shared_ptr<VulkanDevice> vulkanDevice, VkFormat format,VkImageAspectFlags aspect, int32 width, int32 height, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount)
{
    Ref<VulkanTexture> Texture = Create2D(vulkanDevice, nullptr, format, aspect, width, height, usage, sampleCount);

    Texture->DescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    Texture->ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    return Texture;
}

Ref<VulkanTexture> VulkanTexture::CreateAttachment(Ref<VulkanDevice> vulkanDevice, VkFormat Format,VkImageAspectFlags Aspect, int32 Width, int32 Height, VkImageUsageFlags Usage)
{
    Ref<VulkanTexture> texture = Create2D(vulkanDevice, nullptr, Format, Aspect, Width, Height, Usage);
    texture->DescriptorInfo.sampler = VK_NULL_HANDLE;
    texture->DescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    texture->ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    return texture;
}

Ref<VulkanTexture> VulkanTexture::Create2D(const uint8* rgbaData,uint32 size,VkFormat format,int32 width,int32 height,std::shared_ptr<VulkanDevice> vulkanDevice,Ref<VulkanCommandBuffer> cmdBuffer,VkImageUsageFlags imageUsageFlags,ImageLayoutBarrier imageLayout)
{
    int32 MipLevels = Math::FloorToInt(Math::Log2(Math::Max(width,height))) + 1;
    VkDevice device = vulkanDevice->GetInstanceHandle();
    
    const uint32 ImageSize = width * height *  G_PixelFormats[format].blockBytes;
    const uint32 Size =  ImageSize > size ? ImageSize : size;
    
    auto StagingBuffer =  VulkanBuffer::CreateBuffer(vulkanDevice,VK_BUFFER_USAGE_TRANSFER_SRC_BIT, Size); 

    StagingBuffer->Map();
    StagingBuffer->CopyFrom((void*)rgbaData,size);
    StagingBuffer->UnMap();

    uint32 MemoryTypeIndex = 0;
    VkMemoryRequirements MemReqs = {};
    VkMemoryAllocateInfo memAllocInfo;
    ZeroVulkanStruct(memAllocInfo, VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);

    // image info
    VkImage                         image = VK_NULL_HANDLE;
    VkImageView                     imageView = VK_NULL_HANDLE;
    VmaAllocation                   imageVmaAllocation;
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
    ImageCreateInfo.extent          = { (uint32_t)width, (uint32_t)height, 1 };
    ImageCreateInfo.sharingMode     = VK_SHARING_MODE_EXCLUSIVE;
    ImageCreateInfo.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageCreateInfo.usage           = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | imageUsageFlags;
    
    VmaAllocationCreateInfo MemoryInfo{};
    MemoryInfo.usage = VMA_MEMORY_USAGE_AUTO;
    VERIFYVULKANRESULT(vmaCreateImage(vulkanDevice->vma_allocator,&ImageCreateInfo,&MemoryInfo,&image,&imageVmaAllocation,nullptr));
    
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
    texture->Device         = vulkanDevice;
    texture->DescriptorInfo = descriptorInfo;
    texture->Format         = format;
    texture->Height         = height;
    texture->Image          = image;
    texture->ImageLayout    = GetImageLayout(imageLayout);
    texture->ImageSampler   = imageSampler;
    texture->mVmaAllocation = imageVmaAllocation;
    texture->ImageView      = imageView;
    texture->Width          = width;
    texture->MipLevels      = MipLevels;
    texture->LayerCount     = 1;
    texture->IsUpload       = true;

    return texture;
}

Ref<VulkanTexture> VulkanTexture::CreateFrameGraphAliasingTexture(Ref<VulkanDevice> vulkanDevice,Ref<VulkanTexture> AliasingTexture, VkFormat format, int32 width, int32 height,VkImageUsageFlags usage,int32 Mipmap, VkSampleCountFlagBits sampleCount)
{
    VkDevice device = vulkanDevice->GetInstanceHandle();

    // image info
    VkImage                         image = VK_NULL_HANDLE;
    VkImageView                     imageView = VK_NULL_HANDLE;
    VkSampler                       imageSampler = VK_NULL_HANDLE;
    VkDescriptorImageInfo           descriptorInfo = {};
    
    VkImageCreateInfo ImageCreateInfo;
    ZeroVulkanStruct(ImageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
    ImageCreateInfo.imageType       = VK_IMAGE_TYPE_2D;
    ImageCreateInfo.format          = format;
    ImageCreateInfo.mipLevels       = Mipmap;
    ImageCreateInfo.arrayLayers     = 1;
    ImageCreateInfo.samples         = VK_SAMPLE_COUNT_1_BIT;
    ImageCreateInfo.tiling          = VK_IMAGE_TILING_OPTIMAL;
    ImageCreateInfo.extent          = { (uint32_t)width, (uint32_t)height, 1 };
    ImageCreateInfo.sharingMode     = VK_SHARING_MODE_EXCLUSIVE;
    ImageCreateInfo.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageCreateInfo.usage           =  VK_IMAGE_USAGE_SAMPLED_BIT | usage;
    
    VkResult vkResult = vmaCreateAliasingImage(vulkanDevice->vma_allocator,AliasingTexture->mVmaAllocation,&ImageCreateInfo,&image);
    if(vkResult != VK_SUCCESS)
    {
        return nullptr;
    }
    
    VkImageViewCreateInfo viewInfo;
    ZeroVulkanStruct(viewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
    viewInfo.image      = image;
    viewInfo.viewType   = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format     = format;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    
    VkImageAspectFlags AspectFlag = format >= VK_FORMAT_D16_UNORM && format <= VK_FORMAT_D32_SFLOAT_S8_UINT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.aspectMask = AspectFlag;
    viewInfo.subresourceRange.levelCount = Mipmap;
    viewInfo.subresourceRange.layerCount = 1;
    VERIFYVULKANRESULT(vkCreateImageView(device, &viewInfo, VULKAN_CPU_ALLOCATOR, &imageView));
    
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
    samplerInfo.maxLod           = (float)Mipmap;
    samplerInfo.minLod           = 0.0f;
    VERIFYVULKANRESULT(vkCreateSampler(device, &samplerInfo, VULKAN_CPU_ALLOCATOR, &imageSampler));

    descriptorInfo.sampler     = imageSampler;
    descriptorInfo.imageView   = imageView;
    descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    Ref<VulkanTexture> texture   = CreateRef<VulkanTexture>();
    texture->Device         = vulkanDevice;
    texture->DescriptorInfo = descriptorInfo;
    texture->Format         = format;
    texture->Height         = height;
    texture->Image          = image;
    texture->ImageLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
    texture->ImageSampler   = imageSampler;
    texture->ImageView      = imageView;
    texture->Width          = width;
    texture->MipLevels      = Mipmap;
    texture->LayerCount     = 1;
    texture->mVmaAllocation = 0;
    texture->IsUpload       = true;

    return texture;
}

Ref<VulkanTexture> VulkanTexture::CreateFrameGraphTexture(Ref<VulkanDevice> vulkanDevice, VkFormat format, int32 width, int32 height, VkImageUsageFlags usage,int32 Mipmap, VkSampleCountFlagBits sampleCount)
{
    VkDevice device = vulkanDevice->GetInstanceHandle();

    // image info
    VkImage                         image = VK_NULL_HANDLE;
    VkImageView                     imageView = VK_NULL_HANDLE;
    VkSampler                       imageSampler = VK_NULL_HANDLE;
    VmaAllocation                   imageAllocation;
    VkDescriptorImageInfo           descriptorInfo = {};
    
    VkImageCreateInfo ImageCreateInfo;
    ZeroVulkanStruct(ImageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
    ImageCreateInfo.imageType       = VK_IMAGE_TYPE_2D;
    ImageCreateInfo.format          = format;
    ImageCreateInfo.mipLevels       = Mipmap;
    ImageCreateInfo.arrayLayers     = 1;
    ImageCreateInfo.samples         = VK_SAMPLE_COUNT_1_BIT;
    ImageCreateInfo.tiling          = VK_IMAGE_TILING_OPTIMAL;
    ImageCreateInfo.extent          = { (uint32_t)width, (uint32_t)height, 1 };
    ImageCreateInfo.sharingMode     = VK_SHARING_MODE_EXCLUSIVE;
    ImageCreateInfo.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageCreateInfo.usage           =  VK_IMAGE_USAGE_SAMPLED_BIT | usage;

    VmaAllocationCreateInfo memory_info{};
    memory_info.usage = VMA_MEMORY_USAGE_AUTO;
    
    VkResult vkResult = vmaCreateImage(
        vulkanDevice->vma_allocator,
        &ImageCreateInfo,
        &memory_info,
        &image,
        &imageAllocation,
        nullptr
    );
    
    if(vkResult != VK_SUCCESS)
    {
        return nullptr;
    }
    
    VkImageViewCreateInfo viewInfo;
    ZeroVulkanStruct(viewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
    viewInfo.image      = image;
    viewInfo.viewType   = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format     = format;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    
    VkImageAspectFlags AspectFlag = format >= VK_FORMAT_D16_UNORM && format <= VK_FORMAT_D32_SFLOAT_S8_UINT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.aspectMask = AspectFlag;
    viewInfo.subresourceRange.levelCount = Mipmap;
    viewInfo.subresourceRange.layerCount = 1;
    VERIFYVULKANRESULT(vkCreateImageView(device, &viewInfo, VULKAN_CPU_ALLOCATOR, &imageView));
    
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
    samplerInfo.maxLod           = (float)Mipmap;
    samplerInfo.minLod           = 0.0f;
    VERIFYVULKANRESULT(vkCreateSampler(device, &samplerInfo, VULKAN_CPU_ALLOCATOR, &imageSampler));

    descriptorInfo.sampler     = imageSampler;
    descriptorInfo.imageView   = imageView;
    descriptorInfo.imageLayout =  format >= VK_FORMAT_D16_UNORM && format <= VK_FORMAT_D32_SFLOAT_S8_UINT ?VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    Ref<VulkanTexture> texture   = CreateRef<VulkanTexture>();
    texture->Device         = vulkanDevice;
    texture->DescriptorInfo = descriptorInfo;
    texture->Format         = format;
    texture->Height         = height;
    texture->Image          = image;
    texture->ImageLayout    = VK_IMAGE_LAYOUT_UNDEFINED;
    texture->mVmaAllocation = imageAllocation;
    texture->ImageSampler   = imageSampler;
    texture->ImageView      = imageView;
    texture->Width          = width;
    texture->MipLevels      = Mipmap;
    texture->LayerCount     = 1;
    texture->IsUpload       = true;

    return texture;
}



bool VulkanTexture::bIsDepth()
{
    return Format >= VK_FORMAT_D16_UNORM && Format <= VK_FORMAT_D32_SFLOAT_S8_UINT; 
}

Ref<VulkanTexture> VulkanTexture::CreateCube(Ref<VulkanDevice> InVulkanDevice, Ref<VulkanCommandBuffer> CmdBuffer,
    VkFormat Format, VkImageAspectFlags Aspect, int32 Width, int32 Height, bool Mipmap, VkImageUsageFlags Usage,VkSampleCountFlagBits SampleCount, ImageLayoutBarrier ImageLayout)
{
    VkDevice Device = InVulkanDevice->GetInstanceHandle();
    int MipMaps = 1;
    if(Mipmap)
    {
        MipMaps = Math::FloorToInt(Math::Log2(float(Math::Max(Width,Height)))) + 1;
    }

    VkImage                         image = VK_NULL_HANDLE;
    VkImageView                     imageView = VK_NULL_HANDLE;
    VkSampler                       imageSampler = VK_NULL_HANDLE;
    VmaAllocation                   imageAllocation;

    VmaAllocationCreateInfo memory_info{};
    memory_info.usage = VMA_MEMORY_USAGE_AUTO;
    
    VkImageCreateInfo ImageCreatInfo;
    ZeroVulkanStruct(ImageCreatInfo,VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
    ImageCreatInfo.imageType = VK_IMAGE_TYPE_2D; //TextureArray for Cube
    ImageCreatInfo.format = Format;
    ImageCreatInfo.mipLevels = MipMaps;
    ImageCreatInfo.arrayLayers = 6;
    ImageCreatInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    ImageCreatInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ImageCreatInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageCreatInfo.extent = {(uint32_t)Width,(uint32_t)Height,1};
    ImageCreatInfo.usage = Usage;
    ImageCreatInfo.samples = SampleCount;
    ImageCreatInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; //
    
    auto Result = vmaCreateImage(
        InVulkanDevice->vma_allocator,
        &ImageCreatInfo,
        &memory_info,
        &image,
        &imageAllocation,
        nullptr);

    if(Result != VK_SUCCESS)
    {
        RE_ERROR("Can't Create CubeTexture");
        return nullptr;
    }
    
    VkImageViewCreateInfo viewInfo;
    ZeroVulkanStruct(viewInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
    viewInfo.image      = image;
    viewInfo.viewType   = VK_IMAGE_VIEW_TYPE_CUBE; //Note
    viewInfo.format     = Format;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    
    viewInfo.subresourceRange.aspectMask = Aspect;
    viewInfo.subresourceRange.levelCount = Mipmap?MipMaps:1;
    viewInfo.subresourceRange.layerCount = 6;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    
    VERIFYVULKANRESULT(vkCreateImageView(Device, &viewInfo, VULKAN_CPU_ALLOCATOR, &imageView));
    
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
    samplerInfo.maxLod           = (float)Mipmap;
    samplerInfo.minLod           = 0.0f;
    VERIFYVULKANRESULT(vkCreateSampler(Device, &samplerInfo, VULKAN_CPU_ALLOCATOR, &imageSampler));

    VkDescriptorImageInfo Info;
    Info.sampler = imageSampler;
    Info.imageView = imageView;
    Info.imageLayout = GetImageLayout(ImageLayout);
    
    Ref<VulkanTexture> texture   = CreateRef<VulkanTexture>();
    texture->Device         = InVulkanDevice;
    texture->DescriptorInfo = Info;
    texture->Format         = Format;
    texture->Height         = Height;
    texture->Width          = Width;

    texture->mVmaAllocation = imageAllocation;
    texture->ImageSampler   = imageSampler;
    texture->ImageView      = imageView;
    texture->Image          = image;
    texture->MipLevels      = MipMaps;
    texture->LayerCount     = 6;
    texture->Depth          = 1;       
    texture->IsCubeMap      = true;
    texture->IsUpload       = false;

    if(CmdBuffer != nullptr && ImageLayout != ImageLayoutBarrier::Undefined)
    {
        VkImageSubresourceRange subresourceRange = {};
        subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.levelCount     = MipMaps;
        subresourceRange.layerCount     = 6;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.baseMipLevel   = 0;
        
        ImagePipelineBarrier(CmdBuffer->CmdBuffer,texture,ImageLayoutBarrier::Undefined,ImageLayout,subresourceRange);
    }
    else
    {
        ImageLayout = ImageLayoutBarrier::Undefined;
    }

    Info.imageLayout = GetImageLayout(ImageLayout);
    texture->ImageLayout    = GetImageLayout(ImageLayout);
    
    return texture;
}

Ref<VulkanTexture> VulkanTexture::CreateCube(std::vector<std::string> FileNames, Ref<VulkanDevice> InVulkanDevice,Ref<VulkanCommandBuffer> CommandBuffer,
     VkImageAspectFlags Aspect,VkImageUsageFlags Usage, ImageLayoutBarrier imageLayout)
{
    struct ImageInfo
    {
        int32   width  = 0;
        int32   height = 0;
        int32   comp   = 0;
        uint8*  data   = nullptr;
        uint32  size   = 0;
    };

    std::vector<ImageInfo> images(FileNames.size());
    for (int32 i = 0; i < FileNames.size(); ++i)
    {
        uint32 dataSize = 0;
        uint8* dataPtr  = nullptr;
        if (!AssetManager::ReadFile(FileNames[i], dataPtr, dataSize))
        {
            RE_ERROR("Failed load image : {0}", FileNames[i].c_str());
            return nullptr;
        }

        ImageInfo& imageInfo = images[i];
        imageInfo.data = (uint8*)StbImage::LoadFloatFromMemory(dataPtr, dataSize, &imageInfo.width, &imageInfo.height, &imageInfo.comp, 4);
        imageInfo.comp = 4;
        imageInfo.size = imageInfo.width * imageInfo.height * imageInfo.comp * 4;

        delete[] dataPtr;
        dataPtr = nullptr;

        if (!imageInfo.data)
        {
            RE_ERROR("Failed load image : %s", FileNames[i].c_str());
            return nullptr;
        }
    }

    // 图片信息，TextureArray要求尺寸一致
    int32 width     = images[0].width;
    int32 height    = images[0].height;
    int32 numArray  = (int32)images.size();
    VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT; // 可以改
    int32 mipLevels = Math::FloorToInt(Math::Log2((float)Math::Max(width, height))) + 1;

    Ref<VulkanBuffer> StagingBuffer = VulkanBuffer::CreateBuffer(
        InVulkanDevice,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        images[0].size * 6
    );

    StagingBuffer->Map();
    for(int32 i = 0 ; i < images.size() ; ++i)
    {
        uint8* src  = images[i].data;
        uint32 size = images[i].size;
        
        memcpy((uint8*)StagingBuffer->Mapped + size * i , src , size);

        StbImage::Free(src);
    }
    StagingBuffer->UnMap();

    Ref<VulkanTexture> Cube = CreateCube(
        InVulkanDevice,
        CommandBuffer,
        format,
        Aspect,
        width,
        height,
        true,
        Usage,
        VK_SAMPLE_COUNT_1_BIT
    );

    CommandBuffer->Begin();
    CommandBuffer->UploadCubeTextureData(Cube,StagingBuffer,images[0].size);
    
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.levelCount     = 1;
    subresourceRange.layerCount     = numArray;
    subresourceRange.baseMipLevel   = 0;
    subresourceRange.baseArrayLayer = 0;
    ImagePipelineBarrier(CommandBuffer->CmdBuffer,Cube,Cube->mImageLayoutBarrier,ImageLayoutBarrier::TransferSource,subresourceRange);
    
    CommandBuffer->GenerateMipMaps(Cube); 

    subresourceRange.aspectMask   = VK_IMAGE_ASPECT_COLOR_BIT; 
    subresourceRange.levelCount   = mipLevels;
    subresourceRange.layerCount   = numArray;
    subresourceRange.baseMipLevel = 0;
    ImagePipelineBarrier(CommandBuffer->CmdBuffer,Cube,Cube->mImageLayoutBarrier,imageLayout,subresourceRange);

    CommandBuffer->End();
    CommandBuffer->Submit();

    StagingBuffer.reset();

    return Cube;
}

Ref<VulkanTexture> VulkanTexture::CreateCubeRT(Ref<VulkanDevice> InvulkanDevice, VkFormat format,
    VkImageAspectFlags aspect, int32 width, int32 height, VkImageUsageFlags usage, VkSampleCountFlagBits sampleCount)
{
    Ref<VulkanTexture> CubeTexture = CreateCube(InvulkanDevice,nullptr,format,aspect,width,height,false,usage,sampleCount);
    CubeTexture->ImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    CubeTexture->DescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

    return CubeTexture;
}

void VulkanTexture::ImagePipelineBarrier(VkCommandBuffer CommandBuffer, Ref<VulkanTexture> Texture,
    ImageLayoutBarrier Source, ImageLayoutBarrier Dest, const VkImageSubresourceRange& subresourceRange,
    uint32_t SrcQueueFmily, uint32_t DstQueueFamily)
{
    {
        VkImageMemoryBarrier ImageBarrier;
        ZeroVulkanStruct(ImageBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
        ImageBarrier.image = Texture->Image;
        ImageBarrier.subresourceRange    = subresourceRange;
        ImageBarrier.srcQueueFamilyIndex = SrcQueueFmily;
        ImageBarrier.dstQueueFamilyIndex = DstQueueFamily;

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

        Texture->DescriptorInfo.imageLayout = ImageBarrier.newLayout;
        Texture->ImageLayout = ImageBarrier.newLayout;
        Texture->OwnerQueueFamily = ImageBarrier.dstQueueFamilyIndex;
        Texture->mImageLayoutBarrier = Dest;
    }
}

void VulkanTexture::ImagePipelineBarrier(VkCommandBuffer CommandBuffer, Ref<VulkanTexture> Texture,ImageLayoutBarrier Source, ImageLayoutBarrier Dest, uint32_t SrcQueueFmily, uint32_t DstQueueFamily)
{
    VkImageMemoryBarrier ImageBarrier;
    ZeroVulkanStruct(ImageBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
    ImageBarrier.image = Texture->Image;
    ImageBarrier.srcQueueFamilyIndex = SrcQueueFmily;
    ImageBarrier.dstQueueFamilyIndex = DstQueueFamily;
    
    VkImageSubresourceRange SubResourceRange{};
    SubResourceRange.aspectMask = Texture->bIsDepth() ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    SubResourceRange.layerCount = Texture->LayerCount;
    SubResourceRange.levelCount = Texture->MipLevels;
    SubResourceRange.baseArrayLayer = 0;
    SubResourceRange.baseMipLevel = 0;
    
    ImageBarrier.subresourceRange  = SubResourceRange;

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

    Texture->DescriptorInfo.imageLayout = ImageBarrier.newLayout;
    Texture->ImageLayout = ImageBarrier.newLayout;
    Texture->OwnerQueueFamily = ImageBarrier.dstQueueFamilyIndex;
    Texture->mImageLayoutBarrier = Dest;
}

void VulkanTexture::ImagePipelineBarrier(VkCommandBuffer CommandBuffer, VkImage Image, ImageLayoutBarrier Source,
                                         ImageLayoutBarrier Dest, const VkImageSubresourceRange& subresourceRange, uint32_t SrcQueueFmily,
                                         uint32_t DstQueueFamily)
{
    {
        VkImageMemoryBarrier ImageBarrier;
        ZeroVulkanStruct(ImageBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);
        ImageBarrier.image = Image;
        ImageBarrier.subresourceRange    = subresourceRange;
        ImageBarrier.srcQueueFamilyIndex = SrcQueueFmily;
        ImageBarrier.dstQueueFamilyIndex = DstQueueFamily;

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
}

void VulkanTexture::ImagePipelineBarrier(VkCommandBuffer CommandBuffer, Ref<VulkanTexture> Texture,
    ImageLayoutBarrier Source, ImageLayoutBarrier Dest, uint32 BaseMiplevel, uint32 MipCount, bool IsDepth,
    uint32_t SrcQueueFmily ,uint32_t DstQueueFamily )
{
    VkImageMemoryBarrier ImageBarrier;
    ZeroVulkanStruct(ImageBarrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);

    ImageBarrier.image = Texture->Image;
    ImageBarrier.subresourceRange.aspectMask = IsDepth ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    ImageBarrier.subresourceRange.baseArrayLayer    = 0;
    ImageBarrier.subresourceRange.layerCount    = 1;
    ImageBarrier.subresourceRange.levelCount    = MipCount;
    ImageBarrier.subresourceRange.baseMipLevel = BaseMiplevel;
        
    ImageBarrier.srcQueueFamilyIndex = SrcQueueFmily;
    ImageBarrier.dstQueueFamilyIndex = DstQueueFamily;

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

    Texture->ImageLayout = ImageBarrier.newLayout;
    Texture->mImageLayoutBarrier = Dest;
    Texture->DescriptorInfo.imageLayout = ImageBarrier.newLayout;
    Texture->OwnerQueueFamily = ImageBarrier.dstQueueFamilyIndex;
}

VkImageView VulkanTexture::CreateImageView(uint32 BaseMipLevel,uint32 NumMip)
{
    VkImageViewCreateInfo viewCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    viewCreateInfo.image = Image;
    viewCreateInfo.viewType = (LayerCount == 6) ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = Format;
    viewCreateInfo.subresourceRange.aspectMask = bIsDepth() ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;;
    viewCreateInfo.subresourceRange.baseMipLevel = BaseMipLevel;
    viewCreateInfo.subresourceRange.levelCount = NumMip;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    VkImageView view;
    vkCreateImageView(Device->GetInstanceHandle(), &viewCreateInfo, nullptr, &view);
    return view;
}
