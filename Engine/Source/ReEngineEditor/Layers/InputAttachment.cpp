#include "InputAttachMent.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "obj_vert.h"
#include "obj_frag.h"
#include "quad_vert.h"
#include "quad_frag.h"

void InputAttachmentBackBuffer::CreateFrameBuffers()
{
    DestroyFrameBuffers();
    
    int32 fwidth    = g_VulkanInstance->GetSwapChain()->GetWidth();
    int32 fheight   = g_VulkanInstance->GetSwapChain()->GetHeight();
    VkDevice device = g_VulkanInstance->GetDevice()->GetInstanceHandle();

    VkImageView attachments[4];

    VkFramebufferCreateInfo frameBufferCreateInfo;
    ZeroVulkanStruct(frameBufferCreateInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
    frameBufferCreateInfo.renderPass      = m_RenderPass;
    frameBufferCreateInfo.attachmentCount = 4;
    frameBufferCreateInfo.pAttachments    = attachments;
    frameBufferCreateInfo.width			  = fwidth;
    frameBufferCreateInfo.height		  = fheight;
    frameBufferCreateInfo.layers		  = 1;

    const std::vector<VkImageView>& backbufferViews = g_VulkanInstance->GetBackbufferViews();

    m_FrameBuffers.resize(backbufferViews.size());
    for (uint32 i = 0; i < m_FrameBuffers.size(); ++i)
    {
        attachments[0] = backbufferViews[i];
        attachments[1] = (*m_AttachmentColors)[i]->ImageView;
        attachments[2] = (*m_AttachmentNormals)[i]->ImageView;
        attachments[3] = (*m_AttachmentDepth)[i]->ImageView;
        
        VERIFYVULKANRESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, VULKAN_CPU_ALLOCATOR, &m_FrameBuffers[i]));
    }
}

void InputAttachmentBackBuffer::CreateRenderPass()
{
    DestoryRenderPass();

    VkDevice device = g_VulkanInstance->GetDevice()->GetInstanceHandle();
    PixelFormat pixelFormat = g_VulkanInstance->GetPixelFormat();

    std::vector<VkAttachmentDescription> attachments(4);

    // swap chain attachment
    attachments[0].format         = PixelFormatToVkFormat(pixelFormat, false);
    attachments[0].samples        = m_SampleCount;
    attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    // color attachment
    attachments[1].format         = PixelFormatToVkFormat(pixelFormat, false);
    attachments[1].samples        = m_SampleCount;
    attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // normal attachment
    attachments[2].format         = VK_FORMAT_R8G8B8A8_UNORM;
    attachments[2].samples        = m_SampleCount;
    attachments[2].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[2].storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[2].finalLayout    = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    // depth stencil attachment
    attachments[3].format         = PixelFormatToVkFormat(m_DepthFormat, false);
    attachments[3].samples        = m_SampleCount;
    attachments[3].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[3].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[3].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[3].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[3].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorReferences[2];
    colorReferences[0].attachment = 1;
    colorReferences[0].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorReferences[1].attachment = 2;
    colorReferences[1].layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference swapReference = { };
    swapReference.attachment = 0;
    swapReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference = { };
    depthReference.attachment = 3;
    depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference inputReferences[3];
    inputReferences[0].attachment = 1;
    inputReferences[0].layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    inputReferences[1].attachment = 2;
    inputReferences[1].layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    inputReferences[2].attachment = 3;
    inputReferences[2].layout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    std::vector<VkSubpassDescription> subpassDescriptions(2);
    subpassDescriptions[0].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescriptions[0].colorAttachmentCount    = 2;
    subpassDescriptions[0].pColorAttachments       = colorReferences;
    subpassDescriptions[0].pDepthStencilAttachment = &depthReference;

    subpassDescriptions[1].pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescriptions[1].colorAttachmentCount    = 1;
    subpassDescriptions[1].pColorAttachments       = &swapReference;
    subpassDescriptions[1].inputAttachmentCount    = 3;
    subpassDescriptions[1].pInputAttachments       = inputReferences;

    std::vector<VkSubpassDependency> dependencies(3);
    dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass      = 0;
    dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass      = 0;
    dependencies[1].dstSubpass      = 1;
    dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask   = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[2].srcSubpass      = 1;
    dependencies[2].dstSubpass      = VK_SUBPASS_EXTERNAL;
    dependencies[2].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[2].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[2].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[2].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo;
    ZeroVulkanStruct(renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments    = attachments.data();
    renderPassInfo.subpassCount    = subpassDescriptions.size();
    renderPassInfo.pSubpasses      = subpassDescriptions.data();
    renderPassInfo.dependencyCount = dependencies.size();
    renderPassInfo.pDependencies   = dependencies.data();
    VERIFYVULKANRESULT(vkCreateRenderPass(device, &renderPassInfo, VULKAN_CPU_ALLOCATOR, &m_RenderPass));
}


void InputAttachment::OnCreateBackBuffer()
{
    CreateAttachments();
    FrameBuffer = CreateRef<InputAttachmentBackBuffer>(VkContext->WinProperty->Width,VkContext->WinProperty->Height,VkContext->WinProperty->Title.c_str(),&m_AttachmentDepth,&m_AttachmentNormals,&m_AttachmentColors);
    FrameBuffer->Init(VkContext);
}

void InputAttachment::OnInit()
{
    CreateBuffers();
    CreateDescriptor();
    CreatePipeline();
}

void InputAttachment::OnDeInit()
{
    m_Model.reset();
    m_Quad.reset();

    m_Pipeline0.reset();
    m_Shader0.reset();
    m_DescriptorSet0.reset();

    m_Pipeline1.reset();
    m_Shader1.reset();

    m_RingBuffer.reset();
    
    for(auto Index = 0; Index < m_AttachmentColors.size(); Index++)
    {
        m_AttachmentColors[Index].reset();
        m_AttachmentDepth[Index].reset();
        m_AttachmentNormals[Index].reset();
    }

}

void InputAttachment::OnUpdate(Timestep ts)
{
    m_RingBuffer->OnBeginFrame();
    m_Camera->OnUpdate(ts);
    
    m_Camera->SetFarPlane(m_DebugParam.zFar);
    m_Camera->SetNearPlane(m_DebugParam.zNear);
    
    ModelMatrix.model = glm::identity<glm::mat4>();;
    ViewParam.view = m_Camera->GetViewMatrix();
    ViewParam.projection = m_Camera->GetProjection();
}

void InputAttachment::OnRender()
{
    VkCommandBufferBeginInfo cmdBeginInfo;
    ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);

    VkClearValue clearValues[4];
    clearValues[0].color        = {
        { 0.2f, 0.2f, 0.2f, 0.0f }
    };
    clearValues[1].color        = {
        { 0.2f, 0.2f, 0.2f, 0.0f }
    };
    clearValues[2].color        = {
        { 0.2f, 0.2f, 0.2f, 0.0f }
    };
    clearValues[3].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo;
    ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
    renderPassBeginInfo.renderPass      = FrameBuffer->m_RenderPass;
    renderPassBeginInfo.clearValueCount = 4;
    renderPassBeginInfo.pClearValues    = clearValues;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width  = FrameBuffer->m_Width;
    renderPassBeginInfo.renderArea.extent.height = FrameBuffer->m_Height;

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
    renderPassBeginInfo.framebuffer = FrameBuffer->m_FrameBuffers[i];

    VERIFYVULKANRESULT(vkBeginCommandBuffer(VkContext->GetCommandList(), &cmdBeginInfo));
    vkCmdBeginRenderPass(VkContext->GetCommandList(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdSetViewport(VkContext->GetCommandList(), 0, 1, &viewport);
    vkCmdSetScissor(VkContext->GetCommandList(),  0, 1, &scissor);

    // pass0
    {
        const auto ViewBufferView =  m_RingBuffer->AllocConstantBuffer(sizeof(ViewProjectionBlock),&ViewParam);
        vkCmdBindPipeline(VkContext->GetCommandList(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline0->Pipeline);
        for (int32 meshIndex = 0; meshIndex < m_Model->Meshes.size(); ++meshIndex)
        { 
            ModelMatrix.model =  m_Model->Meshes[meshIndex]->LinkNode.lock()->GetGlobalMatrix();
            const auto BufferView = m_RingBuffer->AllocConstantBuffer(sizeof(ModelBlock),&ModelMatrix);
            
            m_DescriptorSet0->WriteBindOffset("uboViewProj",ViewBufferView.offset);
            m_DescriptorSet0->WriteBindOffset("uboModel",BufferView.offset);
            m_DescriptorSet0->BindSet(VkContext->GetCommandList(),m_Pipeline0->PipelineLayout);

            m_Model->Meshes[meshIndex]->BindDraw(VkContext->GetCommandList());
        }
    }

    vkCmdNextSubpass(VkContext->GetCommandList(), VK_SUBPASS_CONTENTS_INLINE);
    
    // // pass1
    {
        auto BufferView = m_RingBuffer->AllocConstantBuffer(sizeof(AttachmentParamBlock),&m_DebugParam);
        const uint32_t UniformOffset[1] = {(uint32_t)BufferView.offset};
        vkCmdBindPipeline(VkContext->GetCommandList(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline1->Pipeline);
        vkCmdBindDescriptorSets(VkContext->GetCommandList(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline1->PipelineLayout, 0, m_DescriptorSets[i]->DescriptorSets.size(), m_DescriptorSets[i]->DescriptorSets.data(), 1, UniformOffset);
        for (int32 meshIndex = 0; meshIndex < m_Quad->Meshes.size(); ++meshIndex)
        {
            m_Quad->Meshes[meshIndex]->BindDraw(VkContext->GetCommandList());
        }
    }

    vkCmdEndRenderPass(VkContext->GetCommandList());
}

void InputAttachment::OnUIRender(Timestep ts)
{
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin("InputAttachmentsDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        ImGui::Text("Color Depth");

        ImGui::Combo("Attachment", &m_DebugParam.attachmentIndex, m_DebugNames.data(), m_DebugNames.size());
        ImGui::SliderFloat("Z-Near", &m_DebugParam.zNear, 0.1f, 3000.0f);
        ImGui::SliderFloat("Z-Far", &m_DebugParam.zFar, 0.1f, 6000.0f);

        if (m_DebugParam.zNear >= m_DebugParam.zFar)
        {
            m_DebugParam.zNear = m_DebugParam.zFar * 0.5f;
        }

        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }
}

void InputAttachment::OnChangeWindowSize(std::shared_ptr<ReEngine::Event> e)
{
   
}

void InputAttachment::CreateAttachments()
{
    auto SwapChain = VkContext->CommandPool->m_SwapChain;
    int32 fWidth = SwapChain->GetWidth();
    int32 fHeight = SwapChain->GetHeight();
    int32 NumberBuffer = SwapChain->GetBackBufferCount();

    m_AttachmentColors.resize(NumberBuffer);
    m_AttachmentDepth.resize(NumberBuffer);
    m_AttachmentNormals.resize(NumberBuffer);

    for (int32 i = 0; i < m_AttachmentColors.size(); ++i)
    {
        m_AttachmentColors[i] = VulkanTexture::CreateAttachment(
            VkContext->GetVulkanInstance()->GetDevice(),
            PixelFormatToVkFormat(VkContext->GetVulkanInstance()->GetPixelFormat(), false),
            VK_IMAGE_ASPECT_COLOR_BIT,
            fWidth,
            fHeight,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
        );
    }

    for (int32 i = 0; i < m_AttachmentNormals.size(); ++i)
    {
        m_AttachmentNormals[i] = VulkanTexture::CreateAttachment(
        VkContext->GetVulkanInstance()->GetDevice(),
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_ASPECT_COLOR_BIT,
        fWidth,
        fHeight,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
        );
    }

    for (int32 i = 0; i < m_AttachmentDepth.size(); ++i)
    {
        m_AttachmentDepth[i] = VulkanTexture::CreateAttachment(
        VkContext->GetVulkanInstance()->GetDevice(),
        PixelFormatToVkFormat(PF_DepthStencil, false),
        VK_IMAGE_ASPECT_DEPTH_BIT,
        fWidth,
        fHeight,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT
        );
    }
    
}

void InputAttachment::CreatePipeline()
{
    auto Device = VkContext->GetVulkanInstance()->GetDevice();
    VulkanPipelineInfo pipelineInfo0;
    pipelineInfo0.Shader = m_Shader0;
    pipelineInfo0.ColorAttachMenst = 2;
    m_Pipeline0 = VulkanPipeline::Create(
        Device,
        VkContext->CommandPool->m_PipelineCache,
        pipelineInfo0,
        {
            m_Model->GetInputBinding()
        },
        m_Model->GetInputAttributes(),
        m_Shader0->pipelineLayout,
        FrameBuffer->m_RenderPass
    );

    VulkanPipelineInfo pipelineInfo1;
    pipelineInfo1.DepthStencilState.depthTestEnable   = VK_FALSE;
    pipelineInfo1.DepthStencilState.depthWriteEnable  = VK_FALSE;
    pipelineInfo1.DepthStencilState.stencilTestEnable = VK_FALSE;
    pipelineInfo1.Shader  = m_Shader1;
    pipelineInfo1.SubPass = 1;
    
    m_Pipeline1 = VulkanPipeline::Create(
        Device,
        VkContext->CommandPool->m_PipelineCache,
        pipelineInfo1,
        {
            m_Quad->GetInputBinding()
        },
        m_Quad->GetInputAttributes(),
        m_Shader1->pipelineLayout,
        FrameBuffer->m_RenderPass
    );
}

void InputAttachment::CreateBuffers()
{
    auto device = VkContext->Instance->GetDevice();
    
    m_Shader0 = VulkanShader::Create(device,true,&OBJ_VERT,&OBJ_FRAG,nullptr,nullptr,nullptr,nullptr);
    m_Shader1 = VulkanShader::Create(device,true,&QUAD_VERT,&QUAD_FRAG,nullptr,nullptr,nullptr,nullptr);
    
    auto cmdBuffer = VulkanCommandBuffer::Create(device, VkContext->CommandPool->m_CommandPool);

    // scene model
    m_Model = VulkanModel::LoadFromFile(
        "Assets/Mesh/Room/miniHouse_FBX.FBX",
        device,
        cmdBuffer,
        m_Shader0->perVertexAttributes
    );

    // quad model
    std::vector<float> vertices = {
        -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f
    };
    std::vector<uint16> indices = {
        0, 1, 2, 0, 2, 3
    };

    m_Quad = VulkanModel::Create(
        device,
        cmdBuffer,
        vertices,
        indices,
        m_Shader1->perVertexAttributes
    );
    
    m_RingBuffer = CreateRef<VulkanDynamicBufferRing>();
    m_RingBuffer->OnCreate(VkContext->Instance->GetDevice(),3,200 * 1024 * 1024);

    m_DebugParam.zNear = 300.0f;
    m_DebugParam.zFar = 3000.0f;

    ModelMatrix.model = glm::identity<glm::mat4>();
    ViewParam.projection = glm::identity<glm::mat4>();
    ViewParam.view = glm::identity<glm::mat4>();
    
    m_DebugNames.push_back("Color");
    m_DebugNames.push_back("Depth");
    m_DebugNames.push_back("Normal");

    m_Camera = CreateRef<EditorCamera>();
}

void InputAttachment::CreateDescriptor()
{
    m_DescriptorSet0 = m_Shader0->AllocateDescriptorSet();
    m_DescriptorSet0->WriteBuffer("uboViewProj", m_RingBuffer->GetSetDescriptor(sizeof(ViewProjectionBlock)));
    m_DescriptorSet0->WriteBuffer("uboModel",m_RingBuffer->GetSetDescriptor(sizeof(ModelBlock)));

    m_DescriptorSets.resize(m_AttachmentColors.size());
    for (int32 i = 0; i < m_DescriptorSets.size(); ++i)
    {
        m_DescriptorSets[i] = m_Shader1->AllocateDescriptorSet();
        m_DescriptorSets[i]->WriteImage("inputColor", m_AttachmentColors[i]);
        m_DescriptorSets[i]->WriteImage("inputNormal", m_AttachmentNormals[i]);
        m_DescriptorSets[i]->WriteImage("inputDepth", m_AttachmentDepth[i]);
        m_DescriptorSets[i]->WriteBuffer("param", m_RingBuffer->GetSetDescriptor(sizeof(AttachmentParamBlock)));
    }
}
