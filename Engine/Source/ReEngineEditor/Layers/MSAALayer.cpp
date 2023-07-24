#include "MSAALayer.h"
#include "SimplestVert_vert.h"
#include "SimplestFrag_frag.h"
#include "Mesh/LineSphere.h"
#include "Platform/Vulkan/VulkanContext.h"

void MSAABackBuffer::CreateFrameBuffers()
{
    // DestroyFrameBuffers();

    int32 fwidth    = g_VulkanInstance->GetSwapChain()->GetWidth();
    int32 fheight   = g_VulkanInstance->GetSwapChain()->GetHeight();
    VkDevice device = g_VulkanInstance->GetDevice()->GetInstanceHandle();
    
    std::vector<VkImageView> Attachments(4);

    Attachments[0] = MSAAColor->ImageView;
    Attachments[2] = MSAADepth->ImageView;
    //Attachments[1] = backbufferViews[i];
    Attachments[3] = NormalDepth->ImageView;
    
    VkFramebufferCreateInfo frameBufferCreateInfo;
    ZeroVulkanStruct(frameBufferCreateInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
    frameBufferCreateInfo.renderPass      = m_RenderPass;
    frameBufferCreateInfo.attachmentCount = Attachments.size();
    frameBufferCreateInfo.pAttachments    = Attachments.data();
    frameBufferCreateInfo.width           = fwidth;
    frameBufferCreateInfo.height          = fheight;
    frameBufferCreateInfo.layers          = 1;

    const std::vector<VkImageView>& backbufferViews = g_VulkanInstance->GetBackbufferViews();

    m_FrameBuffers.resize(backbufferViews.size());
    for (uint32 i = 0; i < m_FrameBuffers.size(); ++i)
    {
        Attachments[1] = backbufferViews[i];
        VERIFYVULKANRESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, VULKAN_CPU_ALLOCATOR, &m_FrameBuffers[i]));
    }
}

void MSAABackBuffer::CreateRenderPass()
{
    DestoryRenderPass();
    
    VkDevice device = g_VulkanInstance->GetDevice()->GetInstanceHandle();
    PixelFormat pixelFormat = g_VulkanInstance->GetPixelFormat();

    std::vector<VkAttachmentDescription> attachments(4);

    //MSAA Color
    attachments[0].format = PixelFormatToVkFormat(pixelFormat, false);
    attachments[0].samples = m_SampleCount;
    attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // color attachment
    attachments[1].format         = PixelFormatToVkFormat(pixelFormat, false);
    attachments[1].samples        = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout    = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;

    // MSAA Depth
    attachments[2].format         = PixelFormatToVkFormat(PF_DepthStencil, false);
    attachments[2].samples        = m_SampleCount;
    attachments[2].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[2].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[2].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    // depth stencil attachment
    attachments[3].format         = PixelFormatToVkFormat(PF_DepthStencil, false);
    attachments[3].samples        = VK_SAMPLE_COUNT_1_BIT;
    attachments[3].loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[3].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[3].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[3].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[3].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReference;
    colorReference.attachment = 0;
    colorReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference resolveReference;
    resolveReference.attachment = 1;
    resolveReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference depthReference;
    depthReference.attachment = 2;
    depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorReference;
    subpass.pResolveAttachments     = &resolveReference; //这里会填写Resolve
    subpass.pDepthStencilAttachment = &depthReference;

    std::vector<VkSubpassDependency> dependencies(2);
    dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass      = 0;
    dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass      = 0;
    dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo;
    ZeroVulkanStruct(renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments    = attachments.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies   = dependencies.data();
    VERIFYVULKANRESULT(vkCreateRenderPass(device, &renderPassInfo, VULKAN_CPU_ALLOCATOR, &m_RenderPass));
}

void MSAABackBuffer::DestroyFrameBuffers()
{
     MSAADepth.reset();
     MSAAColor.reset();
     NormalDepth.reset();
    
    VulkanBackBuffer::DestroyFrameBuffers();
}

void MSAALayer::OnEvent(std::shared_ptr<ReEngine::Event> e)
{
   
}

void MSAALayer::OnUpdate(ReEngine::Timestep ts)
{
   m_RingBuffer->OnBeginFrame();
    m_Camera->OnUpdate(ts);
    
    MVPData.model = glm::identity<glm::mat4>();
    MVPData.view = m_Camera->GetViewMatrix();
    MVPData.projection = m_Camera->GetProjection();
}

void MSAALayer::OnRender()
{
    VkViewport viewport = {};
    viewport.x        = 0;
    viewport.y        = FrameBuffer->m_Height;
    viewport.width    = FrameBuffer->m_Width;
    viewport.height   = -(float)FrameBuffer->m_Height;    // flip y axis
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.extent.width  = FrameBuffer->m_Width;
    scissor.extent.height = FrameBuffer->m_Height;
    scissor.offset.x      = 0;
    scissor.offset.y      = 0;
    
    int32 i = VkContext->GetCurrtIndex();
  
    
    VkCommandBuffer commandBuffer = VkContext->GetCommandList();

    VkCommandBufferBeginInfo cmdBeginInfo;
    ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
    VERIFYVULKANRESULT(vkBeginCommandBuffer(commandBuffer, &cmdBeginInfo));

    std::vector<VkClearValue> clearValues;

    clearValues.resize(3);
    clearValues[0].color        = {
        { 0.2f, 0.2f, 0.2f, 1.0f }
    };
    clearValues[1].color        = {
        { 0.2f, 0.2f, 0.2f, 1.0f }
    };
    clearValues[2].depthStencil = { 1.0f, 0 };
    
    VkRenderPassBeginInfo renderPassBeginInfo;
    ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
    renderPassBeginInfo.renderPass               = FrameBuffer->m_RenderPass;
    renderPassBeginInfo.framebuffer              = FrameBuffer->m_FrameBuffers[i];
    renderPassBeginInfo.clearValueCount          = clearValues.size();
    renderPassBeginInfo.pClearValues             = clearValues.data();
    renderPassBeginInfo.renderArea.offset.x      = 0;
    renderPassBeginInfo.renderArea.offset.y      = 0;
    renderPassBeginInfo.renderArea.extent.width  = FrameBuffer->m_Width;
    renderPassBeginInfo.renderArea.extent.height = FrameBuffer->m_Height;
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer,  0, 1, &scissor);

    
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, MSAAMaterial->mPipeline->Pipeline);
    for (int32 j = 0; j < LineModel->Meshes.size(); ++j)
    {
        MSAAMaterial->SetLocalUniform("uboMVP",&MVPData,sizeof(MVPBlock));
        MSAAMaterial->BindDescriptorSets(VkContext->GetCommandList(), VK_PIPELINE_BIND_POINT_GRAPHICS);
        LineModel->Meshes[j]->BindDraw(commandBuffer);
    }

    vkCmdEndRenderPass(commandBuffer);
}

void MSAALayer::OnUIRender(ReEngine::Timestep ts)
{
   
}

void MSAALayer::OnCreateBackBuffer()
{
    auto SwapChain = VkContext->CommandPool->m_SwapChain;
    int32 fWidth = SwapChain->GetWidth();
    int32 fHeight = SwapChain->GetHeight();
    
    MSAAColorTexture = VulkanTexture::CreateRenderTarget(
        VkContext->GetVulkanInstance()->GetDevice(),
        PixelFormatToVkFormat(VkContext->GetVulkanInstance()->GetPixelFormat(), false),
        VK_IMAGE_ASPECT_COLOR_BIT,
        fWidth,fHeight,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        MSAACount
    );

    MSAADepthTexture = VulkanTexture::CreateRenderTarget(
        VkContext->GetVulkanInstance()->GetDevice(),
        PixelFormatToVkFormat(PF_DepthStencil, false),
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
        fWidth,fHeight,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        MSAACount
    );

    NormalDepth = VulkanTexture::CreateDepthStencil(
        fWidth,fHeight,
        VkContext->GetVulkanInstance()->GetDevice(),
        PF_DepthStencil
    );

    FrameBuffer = CreateRef<MSAABackBuffer>(VkContext->WinProperty->Width,VkContext->WinProperty->Height,VkContext->WinProperty->Title.c_str(),MSAAColorTexture,MSAADepthTexture,NormalDepth,MSAACount,true);
    FrameBuffer->Init(VkContext);
}

void MSAALayer::OnInit()
{
    CreateBuffer();
}

void MSAALayer::OnDeInit()
{
    LineModel.reset();
    LineShader.reset();
    MSAAMaterial.reset();

    m_Camera.reset();
    m_RingBuffer.reset();
    
    
    MSAAColorTexture.reset();
    MSAADepthTexture.reset();
    NormalDepth.reset();
}

void MSAALayer::OnChangeWindowSize(std::shared_ptr<ReEngine::Event> e)
{
   
}

void MSAALayer::CreateBuffer()
{
    auto device = VkContext->Instance->GetDevice();
    auto cmdBuffer = VulkanCommandBuffer::Create(device, VkContext->CommandPool->m_CommandPool);

    m_RingBuffer = CreateRef<VulkanDynamicBufferRing>();
    m_RingBuffer->OnCreate(VkContext->Instance->GetDevice(),3,200 * 1024 * 1024);
    
    auto Sphere  = LineSphere(40,1.0f);
    LineModel = VulkanModel::Create(
    device,cmdBuffer,
    Sphere.GetVertexs(),
    {},
    {VertexAttribute::VA_Position});

    LineShader = VulkanShader::Create(
        device,
        true,
        &SIMPLESTVERT_VERT,
        &SIMPLESTFRAG_FRAG,
        nullptr,nullptr,nullptr,nullptr
    );

    MSAAMaterial = VulkanMaterial::Create(
        device,
        FrameBuffer->m_RenderPass,
        VkContext->CommandPool->m_PipelineCache,
        LineShader,
        m_RingBuffer
    );

    MSAAMaterial->mPipelineInfo.InputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    MSAAMaterial->mPipelineInfo.RasterizationState.cullMode    = VK_CULL_MODE_NONE;
    MSAAMaterial->mPipelineInfo.RasterizationState.lineWidth   = 1.0;
    MSAAMaterial->mPipelineInfo.RasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
    MSAAMaterial->mPipelineInfo.MultisampleState.rasterizationSamples = MSAACount;
    MSAAMaterial->PreparePipeline();

    m_Camera = CreateRef<EditorCamera>();
    
}
