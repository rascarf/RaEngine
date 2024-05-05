#include "VulkanCommandBuffer.h"

#include "VulkanTexture.h"
#include "VulkanBuffer.h"
#include "Math/Math.h"


NewVulkanCommandBuffer::~NewVulkanCommandBuffer()
{
}

void NewVulkanCommandBuffer::Begin()
{
    if (IsBegun)
    {
        return;
    }
    
    IsBegun = true;

    VkCommandBufferBeginInfo cmdBufBeginInfo;
    ZeroVulkanStruct(cmdBufBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
    cmdBufBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(CmdBuffer, &cmdBufBeginInfo);
}

void NewVulkanCommandBuffer::End()
{
    if (!IsBegun)
    {
        return;
    }

    IsBegun = false;
    vkEndCommandBuffer(CmdBuffer);
}

void NewVulkanCommandBuffer::Submit(VkSemaphore* SignalSemaphore)
{
    
}

Ref<NewVulkanCommandBuffer> NewVulkanCommandBuffer::Create(std::shared_ptr<VulkanDevice> vulkanDevice,VkCommandPool commandPool, VkCommandBufferLevel level, Ref<VulkanQueue>  InQueue)
{
    VkDevice device = vulkanDevice->GetInstanceHandle();

    Ref<NewVulkanCommandBuffer> cmdBuffer = CreateRef<NewVulkanCommandBuffer>();
        
    cmdBuffer->m_VulkanDevice = vulkanDevice;
    cmdBuffer->CommandPool  = commandPool;
    cmdBuffer->IsBegun      = false;

    if(InQueue)
    {
        cmdBuffer->queue = InQueue;
    }
    else
    {
        cmdBuffer->queue = vulkanDevice->GetGraphicsQueue();
    }

    VkCommandBufferAllocateInfo cmdBufferAllocateInfo;
    ZeroVulkanStruct(cmdBufferAllocateInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
    cmdBufferAllocateInfo.commandPool = commandPool;
    cmdBufferAllocateInfo.level       = level;
    cmdBufferAllocateInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(device, &cmdBufferAllocateInfo, &(cmdBuffer->CmdBuffer));

    return cmdBuffer;
}

void NewVulkanCommandBuffer::UploadTextureData(Ref<VulkanTexture> DstTexture,void* SrcData,Ref<VulkanBuffer> InStagingBuffer,uint32 StagingBuffer_Offset)
{
    if(!DstTexture)
    {
        RE_ERROR("UploadTextureData Failed : DstTexture is nullptr");    
        return;
    }

    int32 width = DstTexture->Width;
    int32 height = DstTexture->Height;

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel   = 0;
    subresourceRange.levelCount     = DstTexture->MipLevels;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount     = DstTexture->IsCubeMap ? 6 : 1;

    // undefined to TransferDest
    VulkanTexture::ImagePipelineBarrier(CmdBuffer, DstTexture, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::TransferDest, subresourceRange);
    
    VkBufferImageCopy bufferCopyRegion = {};
    bufferCopyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopyRegion.imageSubresource.mipLevel       = 0;
    bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
    bufferCopyRegion.imageSubresource.layerCount     = DstTexture->IsCubeMap ? 6 : 1;
    bufferCopyRegion.imageExtent.width  = width;
    bufferCopyRegion.imageExtent.height = height;
    bufferCopyRegion.imageExtent.depth  = 1;
    
    vkCmdCopyBufferToImage(CmdBuffer, InStagingBuffer->Buffer, DstTexture->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);
    
    VulkanTexture::ImagePipelineBarrier(CmdBuffer, DstTexture, ImageLayoutBarrier::TransferDest, ImageLayoutBarrier::PixelShaderRead, subresourceRange);
    DstTexture->IsUpload = true;
}

void NewVulkanCommandBuffer::UploadCubeTextureData(Ref<VulkanTexture> DstTexture, Ref<VulkanBuffer> StagingBuffer,uint32 SliceSize)
{
    if(!DstTexture)
    {
        RE_ERROR("UploadTextureData Failed : DstTexture is nullptr");    
        return;
    }

    int32 width = DstTexture->Width;
    int32 height = DstTexture->Height;

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel   = 0;
    subresourceRange.levelCount     = DstTexture->MipLevels;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount     = 6;

    // undefined to TransferDest
    VulkanTexture::ImagePipelineBarrier(CmdBuffer, DstTexture, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::TransferDest, subresourceRange);

    std::vector<VkBufferImageCopy> bufferCopyRegions;
    for(int i = 0 ; i < 6 ; i++)
    {
        VkBufferImageCopy SliceCopy = {};
        SliceCopy.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        SliceCopy.imageSubresource.mipLevel       = 0; // 这里可以看作Slice为基准
        SliceCopy.imageSubresource.baseArrayLayer = i;
        SliceCopy.imageSubresource.layerCount     = 1;
        SliceCopy.imageExtent.width  = width;
        SliceCopy.imageExtent.height = height;
        SliceCopy.imageExtent.depth  = 1;
        SliceCopy.bufferOffset = SliceSize * i;
        
        bufferCopyRegions.push_back(SliceCopy);
    }
    
    vkCmdCopyBufferToImage(CmdBuffer, StagingBuffer->Buffer, DstTexture->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, bufferCopyRegions.size(), bufferCopyRegions.data());
    
    VulkanTexture::ImagePipelineBarrier(CmdBuffer, DstTexture, ImageLayoutBarrier::TransferDest, ImageLayoutBarrier::PixelShaderRead, subresourceRange);
    DstTexture->IsUpload = true;
}

void NewVulkanCommandBuffer::GenerateMipMaps(Ref<VulkanTexture> DstTexture)
{
    if(!DstTexture)
    {
        return;
    }

    if(!DstTexture->IsUpload)
    {
        RE_ERROR("DstTexture is null");
    }

    int32 width = DstTexture->Width;
    int32 height = DstTexture->Height;
    
    int32 MipLevels = Math::FloorToInt(Math::Log2(Math::Max(DstTexture->Width,DstTexture->Height))) + 1;

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.levelCount     = DstTexture->MipLevels;
    subresourceRange.layerCount     = DstTexture->IsCubeMap ? 6 : 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.baseMipLevel   = 0;
    
    VulkanTexture::ImagePipelineBarrier(CmdBuffer,DstTexture->Image,ImageLayoutBarrier::Undefined, ImageLayoutBarrier::TransferSource,subresourceRange);
    
    for(uint32_t i = 1 ; i <(uint32_t) MipLevels ; i++)
    {
        VkImageBlit imageBlit = {};
    
        int32 mip0Width  = ReEngine::Math::Max(width >> (i - 1), 1);
        int32 mip0Height = ReEngine::Math::Max(height >> (i - 1), 1);
        int32 mip1Width  = ReEngine::Math::Max(width >> (i - 0), 1);
        int32 mip1Height = ReEngine::Math::Max(height >> (i - 0), 1);
    
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.layerCount = DstTexture->IsCubeMap ? 6 : 1;
        imageBlit.srcSubresource.mipLevel   = i - 1;
        imageBlit.srcOffsets[1].x = int32_t(mip0Width);
        imageBlit.srcOffsets[1].y = int32_t(mip0Height);
        imageBlit.srcOffsets[1].z = 1;
    
        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.layerCount = DstTexture->IsCubeMap ? 6 : 1;
        imageBlit.dstSubresource.mipLevel   = i;
        imageBlit.dstOffsets[1].x = int32_t(mip1Width);
        imageBlit.dstOffsets[1].y = int32_t(mip1Height);
        imageBlit.dstOffsets[1].z = 1;
    
        VkImageSubresourceRange mipSubRange = {};
        mipSubRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        mipSubRange.baseMipLevel   = i;
        mipSubRange.levelCount     = 1;
        mipSubRange.baseArrayLayer = 0;
        mipSubRange.layerCount     = DstTexture->IsCubeMap ? 6 :1;
    
        // undefined to dst
        VulkanTexture::ImagePipelineBarrier(CmdBuffer, DstTexture, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::TransferDest, mipSubRange);
        
        vkCmdBlitImage(CmdBuffer, DstTexture->Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, DstTexture->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);
        
        // dst to src
        VulkanTexture::ImagePipelineBarrier(CmdBuffer, DstTexture, ImageLayoutBarrier::TransferDest, ImageLayoutBarrier::TransferSource, mipSubRange);
    }
}

void NewVulkanCommandBuffer::CopyImage(Ref<VulkanTexture> Src, Ref<VulkanTexture> Dst)
{
    VulkanTexture::ImagePipelineBarrier(CmdBuffer,Src,Src->mImageLayoutBarrier,ImageLayoutBarrier::TransferSource);
    VulkanTexture::ImagePipelineBarrier(CmdBuffer,Dst,Dst->mImageLayoutBarrier,ImageLayoutBarrier::TransferDest);
    
    VkImageCopy copyRegion = {};
    copyRegion.extent.width = Dst->Width;
    copyRegion.extent.height = Dst->Height;
    copyRegion.extent.depth = 1;
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.srcSubresource.layerCount = Dst->LayerCount;
    copyRegion.srcSubresource.mipLevel = 0;
    copyRegion.dstSubresource = copyRegion.srcSubresource;
    vkCmdCopyImage(CmdBuffer,Src->Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,Dst->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,1, &copyRegion);

    VulkanTexture::ImagePipelineBarrier(CmdBuffer,Src,Src->mImageLayoutBarrier,ImageLayoutBarrier::PixelGeneralRW);
    VulkanTexture::ImagePipelineBarrier(CmdBuffer,Dst,Dst->mImageLayoutBarrier,ImageLayoutBarrier::PixelGeneralRW);
}

void NewVulkanCommandBuffer::ReadBackBarrier(Ref<VulkanBuffer> Dst)
{
    VkBufferMemoryBarrier BufferBarrier = {};
    BufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    BufferBarrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT_KHR;
    BufferBarrier.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT_KHR;
    BufferBarrier.buffer = Dst->Buffer;
    BufferBarrier.size  = Dst->Size;
    BufferBarrier.offset = 0;
    
    vkCmdPipelineBarrier(CmdBuffer,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_HOST_BIT,0,0,nullptr,1,&BufferBarrier,0,nullptr);
}

void NewVulkanCommandBuffer::BufferWriteBarrier(Ref<VulkanBuffer> Dst)
{
    VkBufferMemoryBarrier BufferBarrier = {};
    BufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    BufferBarrier.srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT_KHR;
    BufferBarrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT_KHR;
    BufferBarrier.buffer = Dst->Buffer;
    BufferBarrier.size  = Dst->Size;
    BufferBarrier.offset = 0;
    
    vkCmdPipelineBarrier(CmdBuffer,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,0,nullptr,1,&BufferBarrier,0,nullptr);
}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
    vkDestroyFence(m_VulkanDevice.lock()->GetInstanceHandle(),Fence,nullptr);
}

void VulkanCommandBuffer::Begin()
{
    NewVulkanCommandBuffer::Begin();
}

void VulkanCommandBuffer::End()
{
    NewVulkanCommandBuffer::End();
}

void VulkanCommandBuffer::Submit(VkSemaphore* SignalSemaphore)
{
    End();
    
    VkSubmitInfo submitInfo;
    ZeroVulkanStruct(submitInfo, VK_STRUCTURE_TYPE_SUBMIT_INFO);
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &CmdBuffer;
    submitInfo.signalSemaphoreCount = SignalSemaphore ? 1 : 0;
    submitInfo.pSignalSemaphores    = SignalSemaphore;
		
    if (WaitFlags.size() > 0) 
    {
        submitInfo.waitSemaphoreCount = WaitSemaphores.size();
        submitInfo.pWaitSemaphores    = WaitSemaphores.data();
        submitInfo.pWaitDstStageMask  = WaitFlags.data();
    }

    vkResetFences(m_VulkanDevice.lock()->GetInstanceHandle(), 1, &Fence);
    vkQueueSubmit(queue->GetHandle(), 1, &submitInfo, Fence); //TODO 这里可以斟酌下换成多线程提交
    vkWaitForFences(m_VulkanDevice.lock()->GetInstanceHandle(), 1, &Fence, true, ((uint64)	0xffffffffffffffff));
}

Ref<VulkanCommandBuffer> VulkanCommandBuffer::Create(std::shared_ptr<::VulkanDevice> vulkanDevice,VkCommandPool commandPool, VkCommandBufferLevel level,std::shared_ptr<VulkanQueue> InQueue)
{
    VkDevice device = vulkanDevice->GetInstanceHandle();

    Ref<VulkanCommandBuffer> cmdBuffer = CreateRef<VulkanCommandBuffer>();
        
    cmdBuffer->m_VulkanDevice = vulkanDevice;
    cmdBuffer->CommandPool  = commandPool;
    cmdBuffer->IsBegun      = false;

    if(InQueue)
    {
        cmdBuffer->queue = InQueue;
    }
    else
    {
        cmdBuffer->queue = vulkanDevice->GetGraphicsQueue();
    }

    VkCommandBufferAllocateInfo cmdBufferAllocateInfo;
    ZeroVulkanStruct(cmdBufferAllocateInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);
    cmdBufferAllocateInfo.commandPool = commandPool;
    cmdBufferAllocateInfo.level       = level;
    cmdBufferAllocateInfo.commandBufferCount = 1;
    vkAllocateCommandBuffers(device, &cmdBufferAllocateInfo, &(cmdBuffer->CmdBuffer));

    VkFenceCreateInfo fenceCreateInfo;
    ZeroVulkanStruct(fenceCreateInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
    fenceCreateInfo.flags = 0;
    vkCreateFence(device, &fenceCreateInfo, nullptr, &(cmdBuffer->Fence));

    return cmdBuffer;
}
