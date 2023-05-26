#include "InputAttachMent.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "obj_vert.h"
#include "obj_frag.h"
#include "quad_vert.h"
#include "quad_frag.h"
#include "Debug_frag.h"
#include "Ray_frag.h"
#include "Math/Math.h"
#include "Mesh/Quad.h"

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
    
    VkAttachmentReference colorReferences[3];
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
    CreateMaterial();
}

void InputAttachment::OnDeInit()
{
    m_Model.reset();
    m_Quad.reset();
    m_Role.reset();

    m_Material0.reset();
    m_Material1.reset();
    m_RoleMaterial.reset();
    m_EffectMaterial1.reset();

    for(auto& scene:m_SceneMeshes)
    {
        for(auto& Mesh : scene)
        {
            Mesh.reset();
        }
    }

    for(auto& tex: m_ModelDiffuses)
    {
        tex.reset();
    }

    m_RoleDiffuse.reset();
    
    m_Shader0.reset();
    m_Shader1.reset();
    m_EffectShader.reset();
    
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
    UpdateLight(ts);
    
    m_Camera->SetFarPlane(m_DebugParam.zFar);
    m_Camera->SetNearPlane(m_DebugParam.zNear);
    
    ModelMatrix.model = glm::identity<glm::mat4>();
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
        { 0.5f, 0.5f, 0.5f, 0.5f }
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

    const auto ViewBufferView =  m_RingBuffer->AllocConstantBuffer(sizeof(ViewProjectionBlock),&ViewParam);
    // pass0
    {
        //Role
        vkCmdBindPipeline(VkContext->GetCommandList(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_RoleMaterial->mPipeline->Pipeline);
        for (int32 meshIndex = 0; meshIndex < m_Role->Meshes.size(); ++meshIndex)
        {
            ModelMatrix.model =  m_Role->Meshes[meshIndex]->LinkNode.lock()->GetGlobalMatrix();
            m_RoleMaterial->SetTexture("DiffuseMap",m_RoleDiffuse);
            m_RoleMaterial->SetLocalUniform("uboViewProj",ViewBufferView);
            m_RoleMaterial->SetLocalUniform("uboModel",&ModelMatrix,sizeof(ModelBlock));
            m_RoleMaterial->BindDescriptorSets(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS);
            m_Role->Meshes[meshIndex]->BindDraw(VkContext->GetCommandList());
        }

        //Room
        vkCmdBindPipeline(VkContext->GetCommandList(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Material0->mPipeline->Pipeline); 
        for (int32 textureIndex = 0; textureIndex < m_SceneMeshes.size(); ++textureIndex)
        {
            m_Material0->SetTexture("DiffuseMap",m_ModelDiffuses[textureIndex]);
            for(const auto& Meshes : m_SceneMeshes[textureIndex])
            {
                ModelMatrix.model =  Meshes->LinkNode.lock()->GetGlobalMatrix();
            
                m_Material0->SetLocalUniform("uboModel",&ModelMatrix,sizeof(ModelBlock));
                m_Material0->SetLocalUniform("uboViewProj",ViewBufferView);
                m_Material0->BindDescriptorSets(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS);

                Meshes->BindDraw(VkContext->GetCommandList());
            }
        }

        //Ray
        vkCmdBindPipeline(VkContext->GetCommandList(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_EffectMaterial1->mPipeline->Pipeline);
        for (int32 meshIndex = 0; meshIndex < m_Role->Meshes.size(); ++meshIndex)
        { 
            ModelMatrix.model =  m_Role->Meshes[meshIndex]->LinkNode.lock()->GetGlobalMatrix();
            
            m_EffectMaterial1->SetLocalUniform("uboModel",&ModelMatrix,sizeof(ModelBlock));
            m_EffectMaterial1->SetLocalUniform("uboViewProj",ViewBufferView);
            m_EffectMaterial1->SetLocalUniform("rayParam",&m_RayData,sizeof(RayParamBlock));

            m_EffectMaterial1->BindDescriptorSets(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS);

            m_Role->Meshes[meshIndex]->BindDraw(VkContext->GetCommandList());
        }
    }

    vkCmdNextSubpass(VkContext->GetCommandList(), VK_SUBPASS_CONTENTS_INLINE);
    
    // // pass1
    {
        vkCmdBindPipeline(VkContext->GetCommandList(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Material1->mPipeline->Pipeline);
        for (int32 meshIndex = 0; meshIndex < m_Quad->Meshes.size(); ++meshIndex)
        {
            
            m_Material1->SetLocalUniform("param",&m_DebugParam,sizeof(AttachmentParamBlock));
            m_Material1->SetInputAttachment("inputColor",  m_AttachmentColors[VkContext->GetCurrtIndex()]);
            m_Material1->SetInputAttachment("inputNormal", m_AttachmentNormals[VkContext->GetCurrtIndex()]);
            m_Material1->SetInputAttachment("inputDepth",  m_AttachmentDepth[VkContext->GetCurrtIndex()]);
            
            m_Material1->BindDescriptorSets(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS);
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

        
        ImGui::SliderFloat3("Position", (float*)&m_RolePosition, -500.0f, 500.0f);
        ImGui::SliderFloat3("Scale", (float*)&m_RoleScale, 1.0f, 100.0f);
        
        m_Role->RootNode->LocalMatrix = glm::identity<glm::mat4>();
        m_Role->RootNode->LocalMatrix = glm::scale(m_Role->RootNode->LocalMatrix,m_RoleScale);
        m_Role->RootNode->LocalMatrix = glm::translate(m_Role->RootNode->LocalMatrix,m_RolePosition);

        ImGui::ColorEdit3("Color", (float*)&m_RayData.color);
        ImGui::SliderFloat("Pow", &m_RayData.power, 1.0f, 10.0f);

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

void InputAttachment::CreateMaterial()
{
    auto Device = VkContext->GetVulkanInstance()->GetDevice();
    m_Material0 = VulkanMaterial::Create(
        Device,
        FrameBuffer->m_RenderPass,
        VkContext->CommandPool->m_PipelineCache,
        m_Shader0,
        m_RingBuffer
    );
    
    m_Material0->mPipelineInfo.ColorAttachmentsCount = 2;
    VkPipelineDepthStencilStateCreateInfo& depthStencilState = m_Material0->mPipelineInfo.DepthStencilState;
    depthStencilState.stencilTestEnable = VK_TRUE;
    depthStencilState.back.reference    = 1;
    depthStencilState.back.writeMask    = 0xFF;
    depthStencilState.back.compareMask  = 0xFF;
    depthStencilState.back.compareOp    = VK_COMPARE_OP_ALWAYS;
    depthStencilState.back.failOp       = VK_STENCIL_OP_REPLACE;
    depthStencilState.back.depthFailOp  = VK_STENCIL_OP_REPLACE;
    depthStencilState.back.passOp       = VK_STENCIL_OP_REPLACE;
    depthStencilState.front             = depthStencilState.back;
    m_Material0->PreparePipeline();

    m_RoleMaterial = VulkanMaterial::Create(
        Device,
        FrameBuffer->m_RenderPass,
        VkContext->CommandPool->m_PipelineCache,
        m_Shader0,
        m_RingBuffer
    );
    m_RoleMaterial->mPipelineInfo.ColorAttachmentsCount = 2;
    m_RoleMaterial->PreparePipeline();
    
    m_EffectMaterial1 = VulkanMaterial::Create(
        Device,
        FrameBuffer->m_RenderPass,
        VkContext->CommandPool->m_PipelineCache,
        m_EffectShader,
        m_RingBuffer
    );

    m_EffectMaterial1->mPipelineInfo.ColorAttachmentsCount = 2;

    VkPipelineColorBlendAttachmentState& blendState = m_EffectMaterial1->mPipelineInfo.BlendAttachmentStates[0];
    blendState.blendEnable         = VK_TRUE;
    blendState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
    blendState.colorBlendOp        = VK_BLEND_OP_ADD;
    
    VkPipelineDepthStencilStateCreateInfo& RaydepthStencilState = m_EffectMaterial1->mPipelineInfo.DepthStencilState;
    RaydepthStencilState.stencilTestEnable = VK_TRUE;
    RaydepthStencilState.back.reference    = 1;
    RaydepthStencilState.back.writeMask    = 0xFF;
    RaydepthStencilState.back.compareMask  = 0xFF;
    RaydepthStencilState.back.compareOp    = VK_COMPARE_OP_EQUAL;
    RaydepthStencilState.back.failOp       = VK_STENCIL_OP_KEEP;
    RaydepthStencilState.back.depthFailOp  = VK_STENCIL_OP_KEEP;
    RaydepthStencilState.back.passOp       = VK_STENCIL_OP_REPLACE;
    RaydepthStencilState.front             = depthStencilState.back;
    RaydepthStencilState.depthTestEnable   = VK_FALSE;
    RaydepthStencilState.depthWriteEnable   = VK_FALSE;
    
    m_EffectMaterial1->PreparePipeline();
    
    m_Material1 = VulkanMaterial::Create( 
        Device,
        FrameBuffer->m_RenderPass,
        VkContext->CommandPool->m_PipelineCache,
        m_Shader1, 
        m_RingBuffer
    ); 

    m_Material1->mPipelineInfo.DepthStencilState.depthTestEnable   = VK_FALSE;
    m_Material1->mPipelineInfo.DepthStencilState.depthWriteEnable  = VK_FALSE;
    m_Material1->mPipelineInfo.DepthStencilState.stencilTestEnable = VK_FALSE;
    m_Material1->mPipelineInfo.Shader  = m_Shader1;
    m_Material1->mPipelineInfo.SubPass = 1;
    m_Material1->PreparePipeline();
}

void InputAttachment::UpdateLight(Timestep ts)
{
    // CurrentTime += ts.GetSeconds();
    // for (int32 i = 0; i < NUM_LIGHTS; ++i)
    // { 
    //     float bias = Math::Sin(CurrentTime * LightInfos.Speed[i]) / 5.0f;
    //     LightDatas.Lights[i].Position.x = LightInfos.Position[i].x + bias * LightInfos.Direction[i].x * 500.0f;
    //     LightDatas.Lights[i].Position.y = LightInfos.Position[i].y + bias * LightInfos.Direction[i].y * 500.0f;
    //     LightDatas.Lights[i].Position.z = LightInfos.Position[i].z + bias * LightInfos.Direction[i].z * 500.0f;
    // }
}

void InputAttachment::CreateBuffers()
{
    auto device = VkContext->Instance->GetDevice();
    
    m_Shader0 = VulkanShader::Create(device,true,&OBJ_VERT,&OBJ_FRAG,nullptr,nullptr,nullptr,nullptr);
    m_Shader1 = VulkanShader::Create(device,true,&QUAD_VERT,&DEBUG_FRAG,nullptr,nullptr,nullptr,nullptr);
    m_EffectShader = VulkanShader::Create(device,true,&OBJ_VERT,&RAY_FRAG,nullptr,nullptr,nullptr,nullptr);
    
    auto cmdBuffer = VulkanCommandBuffer::Create(device, VkContext->CommandPool->m_CommandPool);

    // scene model
    m_Model = VulkanModel::LoadFromFile(
        "Assets/Mesh/Room/miniHouse_FBX.FBX",
        device,
        cmdBuffer,
        m_Shader0->perVertexAttributes
    );

    std::vector<std::string> diffusePaths = {
        "Assets/Mesh/Room/miniHouse_Part1.jpg",
        "Assets/Mesh/Room/miniHouse_Part2.jpg",
        "Assets/Mesh/Room/miniHouse_Part3.jpg",
        "Assets/Mesh/Room/miniHouse_Part4.jpg"
    };

    m_ModelDiffuses.resize(diffusePaths.size());
    for(int32 index = 0 ; index < diffusePaths.size();index++)
    {
        m_ModelDiffuses[index] = VulkanTexture::Create2D(
            diffusePaths[index],
            device,
            cmdBuffer
        );
    }

    m_SceneMeshes.resize(diffusePaths.size());
    for(const auto& Mesh : m_Model->Meshes)
    {
        const std::string& DiffuseName = Mesh->Material.Diffuse;
        if (DiffuseName == "miniHouse_Part1")
        {
            m_SceneMeshes[0].push_back(Mesh);
        }
        else if (DiffuseName == "miniHouse_Part2")
        {
            m_SceneMeshes[1].push_back(Mesh);
        }
        else if (DiffuseName == "miniHouse_Part3")
        {
            m_SceneMeshes[2].push_back(Mesh);
        }
        else if (DiffuseName == "miniHouse_Part4")
        {
            m_SceneMeshes[3].push_back(Mesh);
        }
    }
    
    // role modle
    m_Role = VulkanModel::LoadFromFile(
        "Assets/Mesh/LizardMage/LizardMage_Lowpoly.obj",
        device,
        cmdBuffer,
        m_Shader0->perVertexAttributes
    );
    m_RoleDiffuse = VulkanTexture::Create2D(
            "Assets/Mesh/LizardMage/Body_colors1.jpg",
            device,
            cmdBuffer
    );
    
    // quad model
    Quad DebugQuad;

    m_Quad = VulkanModel::Create(
        device,
        cmdBuffer,
        DebugQuad.GetVertexs(),
        DebugQuad.GetIndices(),
        m_Shader1->perVertexAttributes
    );
    
    m_RingBuffer = CreateRef<VulkanDynamicBufferRing>();
    m_RingBuffer->OnCreate(VkContext->Instance->GetDevice(),3,200 * 1024 * 1024);
    
    m_DebugParam.zNear = 0.1f;
    m_DebugParam.zFar = 1000.0f;

    m_DebugParam.attachmentIndex = 0;

    ModelMatrix.model = glm::identity<glm::mat4>();
    ViewParam.projection = glm::identity<glm::mat4>();
    ViewParam.view = glm::identity<glm::mat4>();
    
    m_DebugNames.push_back("Color");
    m_DebugNames.push_back("Depth");
    m_DebugNames.push_back("Normal");
    m_DebugNames.push_back("Position"); 

    m_Camera = CreateRef<EditorCamera>();

    m_RayData.viewDir = -m_Camera->GetForwardDirection();
    m_RayData.color   = glm::vec3(0.0f, 0.6f, 1.0f);
    m_RayData.power   = 5.0f;
    m_RayData.padding = 0;

    auto Bounds = m_Model->RootNode->GetBounds();
    
    // for (int32 i = 0; i < NUM_LIGHTS; ++i)
    // {
    //     LightDatas.Lights[i].Position.x = Math::RandRange(Bounds.Min.x, Bounds.Max.x);
    //     LightDatas.Lights[i].Position.y = Math::RandRange(Bounds.Min.y, Bounds.Max.y);
    //     LightDatas.Lights[i].Position.z = Math::RandRange(Bounds.Min.z, Bounds.Max.z);
    //     
    //     LightDatas.Lights[i].Color.x = Math::RandRange(0.0f, 1.0f);
    //     LightDatas.Lights[i].Color.y = Math::RandRange(0.0f, 1.0f);
    //     LightDatas.Lights[i].Color.z = Math::RandRange(0.0f, 1.0f);
    //     LightDatas.Lights[i].Radius = Math::RandRange(50.0f, 200.0f);
    //
    //     LightInfos.Position[i]  = LightDatas.Lights[i].Position;
    //     LightInfos.Direction[i] = LightInfos.Position[i];
    //     LightInfos.Direction[i] = glm::normalize(LightInfos.Direction[i]);
    //     LightInfos.Speed[i] = 1.0f + Math::RandRange(0.0f, 5.0f);
    // }
}
