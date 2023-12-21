#include "VulkanRenderTarget.h"

VulkanRenderTargetLayout::VulkanRenderTargetLayout(const VulkanRenderPassInfo& RenderPassInfo)
{
    memset(ColorReferences, 0, sizeof(ColorReferences));
    memset(&DepthStencilReference, 0, sizeof(DepthStencilReference));
    memset(ResolveReferences, 0, sizeof(ResolveReferences));
    memset(InputAttachments, 0, sizeof(InputAttachments));
    memset(Descriptions, 0, sizeof(Descriptions));
    memset(&extent3D, 0, sizeof(extent3D));

    for (int32 index = 0; index < RenderPassInfo.NumColorRenderTargets; ++index)
    {
        const VulkanRenderPassInfo::ColorEntry& colorEntry = RenderPassInfo.ColorRenderTargets[index];
        Ref<VulkanTexture> Texture = colorEntry.RenderTarget;

        extent3D.width  = Texture->Width;
        extent3D.height = Texture->Height;
        extent3D.depth  = Texture->Depth;
        NumSamples      = Texture->NumSamples;

        VkAttachmentDescription& attchmentDescription = Descriptions[NumAttachmentDescriptions];
        attchmentDescription.samples        = NumSamples;
        attchmentDescription.format         = Texture->Format;
        attchmentDescription.loadOp         = colorEntry.LoadAction;
        attchmentDescription.storeOp        = colorEntry.StoreAction;
        attchmentDescription.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attchmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attchmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attchmentDescription.finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        ColorReferences[NumColorAttachments].attachment = NumAttachmentDescriptions;
        ColorReferences[NumColorAttachments].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        if (NumSamples != VK_SAMPLE_COUNT_1_BIT)
        {
            Descriptions[NumAttachmentDescriptions + 1] = Descriptions[NumAttachmentDescriptions];
            Descriptions[NumAttachmentDescriptions + 1].samples = VK_SAMPLE_COUNT_1_BIT;

            ResolveReferences[NumColorAttachments].attachment = NumAttachmentDescriptions + 1;
            ResolveReferences[NumColorAttachments].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            NumAttachmentDescriptions += 1;
            HasResolveAttachments      = true;
        }

        NumAttachmentDescriptions += 1;
        NumColorAttachments       += 1;
    }

    if (RenderPassInfo.DepthStencilRenderTarget.DepthStencilTarget)
    {
        Ref<VulkanTexture> texture = RenderPassInfo.DepthStencilRenderTarget.DepthStencilTarget;
        VkAttachmentDescription& attchmentDescription = Descriptions[NumAttachmentDescriptions];

        extent3D.width  = texture->Width;
        extent3D.height = texture->Height;
        extent3D.depth  = texture->Depth;
        NumSamples      = texture->NumSamples;

        attchmentDescription.samples        = texture->NumSamples;
        attchmentDescription.format         = texture->Format;
        attchmentDescription.loadOp         = RenderPassInfo.DepthStencilRenderTarget.LoadAction;
        attchmentDescription.stencilLoadOp  = RenderPassInfo.DepthStencilRenderTarget.LoadAction;
        attchmentDescription.storeOp        = RenderPassInfo.DepthStencilRenderTarget.StoreAction;
        attchmentDescription.stencilStoreOp = RenderPassInfo.DepthStencilRenderTarget.StoreAction;
        attchmentDescription.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attchmentDescription.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        if(RenderPassInfo.DepthStencilRenderTarget.LoadAction == VK_ATTACHMENT_LOAD_OP_LOAD)
        {
            attchmentDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        
        DepthStencilReference.attachment = NumAttachmentDescriptions;
        DepthStencilReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        NumAttachmentDescriptions += 1;
        HasDepthStencil            = true;
    }

    multiview = RenderPassInfo.MultiView;
    NumUsedClearValues = NumAttachmentDescriptions;
}

uint16 VulkanRenderTargetLayout::SetupSubpasses(VkSubpassDescription* OutDescs, uint32 MaxDescs,VkSubpassDependency* OutDeps, uint32 MaxDeps, uint32& OutNumDependencies) const
{
    memset(OutDescs, 0, sizeof(OutDescs[0]) * MaxDescs);

    OutDescs[0].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    OutDescs[0].colorAttachmentCount    = NumColorAttachments;
    OutDescs[0].pColorAttachments       = NumColorAttachments > 0 ? ColorReferences : nullptr;
    OutDescs[0].pResolveAttachments     = HasResolveAttachments ? ResolveReferences : nullptr;
    OutDescs[0].pDepthStencilAttachment = HasDepthStencil ? &DepthStencilReference : nullptr;

    OutNumDependencies = 0;
    
    return 1;
}

VulkanRenderPass::VulkanRenderPass(VkDevice InDevice, const VulkanRenderTargetLayout& RtLayout):Device(InDevice),Layout(RtLayout)
{
    //TODO 这里不应该只能有一个SubPass
    VkSubpassDescription SubpassDesc = {};
    
    SubpassDesc.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    SubpassDesc.colorAttachmentCount    = RtLayout.NumColorAttachments;
    SubpassDesc.pColorAttachments       = RtLayout.NumColorAttachments > 0 ? RtLayout.ColorReferences : nullptr;
    SubpassDesc.pResolveAttachments     = RtLayout.HasResolveAttachments ? RtLayout.ResolveReferences : nullptr;
    SubpassDesc.pDepthStencilAttachment = RtLayout.HasDepthStencil ? &RtLayout.DepthStencilReference : nullptr;
    SubpassDesc.pInputAttachments       = RtLayout.NumInputAttachments > 0 ? RtLayout.InputAttachments : nullptr; 
    
    VkRenderPassCreateInfo renderPassCreateInfo;
    ZeroVulkanStruct(renderPassCreateInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
    renderPassCreateInfo.attachmentCount = RtLayout.NumAttachmentDescriptions;
    renderPassCreateInfo.pAttachments    = RtLayout.Descriptions;
    renderPassCreateInfo.subpassCount    = 1;
    renderPassCreateInfo.pSubpasses      = &SubpassDesc;
    // renderPassCreateInfo.dependencyCount = NumDependencies;
    // renderPassCreateInfo.pDependencies   = SubpasssDep;
    
    
    if (RtLayout.extent3D.depth > 1 && RtLayout.multiview)
    {
        uint32 MultiviewMask = (0b1 << RtLayout.extent3D.depth) - 1;

        const uint32_t ViewMask[2]     = { MultiviewMask, MultiviewMask };
        const uint32_t CorrelationMask = MultiviewMask;

        VkRenderPassMultiviewCreateInfo multiviewCreateInfo;
        ZeroVulkanStruct(multiviewCreateInfo, VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO);
        multiviewCreateInfo.pNext                = nullptr;
        multiviewCreateInfo.subpassCount         = 1;
        multiviewCreateInfo.pViewMasks           = ViewMask;
        multiviewCreateInfo.dependencyCount      = 0;
        multiviewCreateInfo.pViewOffsets         = nullptr;
        multiviewCreateInfo.correlationMaskCount = 1;
        multiviewCreateInfo.pCorrelationMasks    = &CorrelationMask;

        renderPassCreateInfo.pNext = &multiviewCreateInfo;
    }

    VERIFYVULKANRESULT(vkCreateRenderPass(InDevice, &renderPassCreateInfo, VULKAN_CPU_ALLOCATOR, &RenderPass));
}

VulkanFrameBuffer::VulkanFrameBuffer(VkDevice InDevice,const VulkanRenderTargetLayout& RtLayout,const VulkanRenderPass& RenderPass,const VulkanRenderPassInfo& RenderPassInfo)
{
    Device = InDevice;
    
    //可能包含多Resolve
    NumColorAttachments = RtLayout.NumColorAttachments;
    NumColorRenderTargets = RenderPassInfo.NumColorRenderTargets;
    
    for (int32 index = 0; index < NumColorRenderTargets; ++index)
    {
        // TODO:MSAAView
        Ref<VulkanTexture> Texture = RenderPassInfo.ColorRenderTargets[index].RenderTarget;
        ColorRenderTargetImages[index] = Texture->Image;
        AttachmentTextureViews.push_back(Texture->ImageView);
    }

    if (RtLayout.HasDepthStencil)
    {
        Ref<VulkanTexture> texture = RenderPassInfo.DepthStencilRenderTarget.DepthStencilTarget;
        DepthStencilRenderTargetImage = texture->Image;
        AttachmentTextureViews.push_back(texture->ImageView);
    }

    VkFramebufferCreateInfo frameBufferCreateInfo;
    ZeroVulkanStruct(frameBufferCreateInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
    frameBufferCreateInfo.renderPass      = RenderPass.RenderPass;
    frameBufferCreateInfo.attachmentCount = (uint32_t)AttachmentTextureViews.size();
    frameBufferCreateInfo.pAttachments    = AttachmentTextureViews.data();
    frameBufferCreateInfo.width           = RtLayout.extent3D.width;
    frameBufferCreateInfo.height          = RtLayout.extent3D.height;
    frameBufferCreateInfo.layers          = RtLayout.multiview ? 1 : RtLayout.extent3D.depth;
    VERIFYVULKANRESULT(vkCreateFramebuffer(Device, &frameBufferCreateInfo, VULKAN_CPU_ALLOCATOR, &FrameBuffer));

    Extent2D.width  = RtLayout.extent3D.width;
    Extent2D.height = RtLayout.extent3D.height;
}

void VulkanRenderTarget::BeginRenderPass(VkCommandBuffer CmdBuffer)
{
     for (int32 index = 0; index < RenderPassInfo.NumColorRenderTargets; ++index)
    {
            Ref<VulkanTexture> texture = RenderPassInfo.ColorRenderTargets[index].RenderTarget;
            VkImage image = texture->Image;
            VkImageSubresourceRange subResRange = { };
            subResRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            subResRange.baseMipLevel   = 0;
            subResRange.levelCount     = 1;
            subResRange.layerCount     = texture->Depth;
            subResRange.baseArrayLayer = 0;
            ImagePipelineBarrier(CmdBuffer, image, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::ColorAttachment, subResRange);
    }

    if (RenderPassInfo.DepthStencilRenderTarget.DepthStencilTarget)
    {
        Ref<VulkanTexture> texture = RenderPassInfo.DepthStencilRenderTarget.DepthStencilTarget;
        VkImage image = texture->Image;
        VkImageSubresourceRange subResRange = { };
        subResRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        subResRange.baseMipLevel   = 0;
        subResRange.levelCount     = 1;
        subResRange.layerCount     = RenderPassInfo.DepthStencilRenderTarget.DepthStencilTarget->Depth;
        subResRange.baseArrayLayer = 0;
        ImagePipelineBarrier(CmdBuffer, image, ImageLayoutBarrier::Undefined, ImageLayoutBarrier::DepthStencilAttachment, subResRange);
    }

    VkViewport viewport = {};
    viewport.x        = 0;
    viewport.y        = (float)Extent2D.height;
    viewport.width    = (float)Extent2D.width;
    viewport.height   = -(float)Extent2D.height;    // flip y axis
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.extent.width  = Extent2D.width;
    scissor.extent.height = Extent2D.height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;

    VkRenderPassBeginInfo renderPassBeginInfo;
    ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
    renderPassBeginInfo.renderPass               = mRenderPass->RenderPass;
    renderPassBeginInfo.framebuffer              = mFrameBuffer->FrameBuffer;
    renderPassBeginInfo.renderArea.offset.x      = 0;
    renderPassBeginInfo.renderArea.offset.y      = 0;
    renderPassBeginInfo.renderArea.extent.width  = Extent2D.width;
    renderPassBeginInfo.renderArea.extent.height = Extent2D.height;
    renderPassBeginInfo.clearValueCount          = (uint32_t)ClearValues.size();
    renderPassBeginInfo.pClearValues             = ClearValues.data();
    vkCmdBeginRenderPass(CmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdSetViewport(CmdBuffer, 0, 1, &viewport);
    vkCmdSetScissor(CmdBuffer,  0, 1, &scissor);
}

void VulkanRenderTarget::EndRenderPass(VkCommandBuffer CmdBuffer)
{
    vkCmdEndRenderPass(CmdBuffer);

    for (int32 index = 0; index < RenderPassInfo.NumColorRenderTargets; ++index)
    {
        Ref<VulkanTexture> texture = RenderPassInfo.ColorRenderTargets[index].RenderTarget;
        VkImage image = texture->Image;
        VkImageSubresourceRange subResRange = { };
        subResRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        subResRange.baseMipLevel   = 0;
        subResRange.levelCount     = 1;
        subResRange.layerCount     = texture->Depth;
        subResRange.baseArrayLayer = 0;
        ImagePipelineBarrier(CmdBuffer, image, ImageLayoutBarrier::ColorAttachment, colorLayout, subResRange);
    }

    if (RenderPassInfo.DepthStencilRenderTarget.DepthStencilTarget)
    {
        Ref<VulkanTexture> texture = RenderPassInfo.DepthStencilRenderTarget.DepthStencilTarget;
        VkImage image = texture->Image;
        VkImageSubresourceRange subResRange = { };
        subResRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        subResRange.baseMipLevel   = 0;
        subResRange.levelCount     = 1;
        subResRange.layerCount     = RenderPassInfo.DepthStencilRenderTarget.DepthStencilTarget->Depth;
        subResRange.baseArrayLayer = 0;
        ImagePipelineBarrier(CmdBuffer, image, ImageLayoutBarrier::DepthStencilAttachment, depthLayout, subResRange);
    }
}

Ref<VulkanRenderTarget> VulkanRenderTarget::Create(Ref<VulkanDevice> VulkanDevice,const VulkanRenderPassInfo& RenderPassInfo)
{
    VkDevice device = VulkanDevice->GetInstanceHandle();

    Ref<VulkanRenderTarget> OutRenderTarget = CreateRef<VulkanRenderTarget>(RenderPassInfo);
    OutRenderTarget->InDevice = device;
    OutRenderTarget->mRenderPass = CreateRef<VulkanRenderPass>(device,OutRenderTarget->RtLayout);
    OutRenderTarget->mFrameBuffer = CreateRef<VulkanFrameBuffer>(device,OutRenderTarget->RtLayout,*OutRenderTarget->mRenderPass,RenderPassInfo);
    OutRenderTarget->Extent2D        = OutRenderTarget->mFrameBuffer->Extent2D;
    
    return OutRenderTarget;
}

Ref<VulkanRenderTarget> VulkanRenderTarget::Create(Ref<VulkanDevice> VulkanDevice,const VulkanRenderPassInfo& RenderPassInfo, glm::vec4 ClearColor)
{
    VkDevice device = VulkanDevice->GetInstanceHandle();

    Ref<VulkanRenderTarget> OutRenderTarget = CreateRef<VulkanRenderTarget>(RenderPassInfo,ClearColor);
    OutRenderTarget->InDevice = device;
    OutRenderTarget->mRenderPass = CreateRef<VulkanRenderPass>(device,OutRenderTarget->RtLayout);
    OutRenderTarget->mFrameBuffer = CreateRef<VulkanFrameBuffer>(device,OutRenderTarget->RtLayout,*OutRenderTarget->mRenderPass,RenderPassInfo);
    OutRenderTarget->Extent2D        = OutRenderTarget->mFrameBuffer->Extent2D;
    
    return OutRenderTarget;
}
