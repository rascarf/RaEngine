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
    subresourceRange.levelCount     = 1;
    subresourceRange.layerCount     = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.baseMipLevel   = 0;

    // undefined to TransferDest
    VulkanTexture::ImagePipelineBarrier(CmdBuffer, DstTexture, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::TransferDest, subresourceRange);
    
    VkBufferImageCopy bufferCopyRegion = {};
    bufferCopyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    bufferCopyRegion.imageSubresource.mipLevel       = 0;
    bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
    bufferCopyRegion.imageSubresource.layerCount     = 1;
    bufferCopyRegion.imageExtent.width  = width;
    bufferCopyRegion.imageExtent.height = height;
    bufferCopyRegion.imageExtent.depth  = 1;
    
    vkCmdCopyBufferToImage(CmdBuffer, InStagingBuffer->Buffer, DstTexture->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);
    
    VulkanTexture::ImagePipelineBarrier(CmdBuffer, DstTexture, ImageLayoutBarrier::TransferDest, ImageLayoutBarrier::PixelShaderRead, subresourceRange);
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
    
    for(uint32_t i = 1 ; i <(uint32_t) MipLevels ; i++)
    {
        VkImageBlit imageBlit = {};
    
        int32 mip0Width  = ReEngine::Math::Max(width >> (i - 1), 1);
        int32 mip0Height = ReEngine::Math::Max(height >> (i - 1), 1);
        int32 mip1Width  = ReEngine::Math::Max(width >> (i - 0), 1);
        int32 mip1Height = ReEngine::Math::Max(height >> (i - 0), 1);
    
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
        VulkanTexture::ImagePipelineBarrier(CmdBuffer, DstTexture, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::TransferDest, mipSubRange);
    
        // blit image
        vkCmdBlitImage(CmdBuffer, DstTexture->Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, DstTexture->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);
    
        // dst to src
        VulkanTexture::ImagePipelineBarrier(CmdBuffer, DstTexture, ImageLayoutBarrier::TransferDest, ImageLayoutBarrier::TransferSource, mipSubRange);
    }
    
    // VulkanTexture::ImagePipelineBarrier(CmdBuffer, DstTexture, DstTexture->mImageLayoutBarrier, ImageLayoutBarrier::TransferSource,0,MipLevels,false,DstTexture->OwnerQueueFamily,queue->GetFamilyIndex());
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
