#include "AnimationTextureLayer.h"

#include <obj_frag.h>
#include <obj_vert.h>
#include <quad_vert.h>
#include <AnimObj_vert.h>
#include <AnimObjPack_vert.h>
#include <AnimObj_frag.h>
#include <FilterPixelation_frag.h>
#include <ColorFilter_frag.h>
#include <AnimationTexture_vert.h>

#include "Mesh/Quad.h"
#include "Platform/Vulkan/VulkanContext.h"

void AnimationTextureLayer::OnCreateBackBuffer()
{
    GraphicalLayer::OnCreateBackBuffer();
}

void AnimationTextureLayer::OnInit()
{
    CreateRenderTarget();
    LoadAsset();
}

void AnimationTextureLayer::OnDeInit()
{
    m_Camera.reset();        

    AnimTexture.reset();

    SceneModel.reset();
    mQuad.reset();

    for(auto& tex : TextureArray)
    {
        tex.reset();
    }
    
    ColorRT.reset();
    DepthRT.reset();

    RenderTarget.reset();
    SceneShader.reset();
    mFilterShader.reset();
    
    SceneMaterial.reset();
    mFilterMaterial.reset();
    
    m_RingBuffer.reset();
}

void AnimationTextureLayer::OnUpdate(Timestep ts)
{
    m_RingBuffer->OnBeginFrame();
    m_Camera->OnUpdate(ts);
    
    SceneModel->UpdateAnimation(ts.GetSeconds());
    
    m_MVPData.animIndex.x = 64;
    m_MVPData.animIndex.y = 64;
    m_MVPData.animIndex.z = SceneModel->AnimationIndex() * SceneModel->Meshes[0]->Bones.size() * 4;
    m_MVPData.animIndex.w = 0;
    
    SceneModel->RootNode->LocalMatrix = glm::rotate(SceneModel->RootNode->LocalMatrix,ts.GetSeconds() *  glm::radians(45.0f),glm::vec3(0.0f, 1.0f, 0.0f));
    
    m_Camera->SetFarPlane(DebugParam.zFar);
    m_Camera->SetNearPlane(DebugParam.zNear);
    
    m_MVPData.view = m_Camera->GetViewMatrix();
    m_MVPData.projection = m_Camera->GetProjection();
}

void AnimationTextureLayer::OnRender()
{
    VkCommandBufferBeginInfo cmdBeginInfo;
    ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
    VERIFYVULKANRESULT(vkBeginCommandBuffer(VkContext->GetCommandList(), &cmdBeginInfo));
    
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
    
    
    RenderTarget->BeginRenderPass(VkContext->GetCommandList());
    
    vkCmdBindPipeline(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS,SceneMaterial->mPipeline->Pipeline);
    
    {
        SceneMaterial->SetTexture("DiffuseMap",TextureArray[0]); 

        for(auto& Mesh :SceneModel->Meshes )
        {
            m_MVPData.model = Mesh->LinkNode.lock()->GetGlobalMatrix();
            
            SceneMaterial->SetLocalUniform("uboMVP",&m_MVPData,sizeof(ModelViewProjectionBlock));
            SceneMaterial->SetTexture("BonesTexture",AnimTexture);
            SceneMaterial->BindDescriptorSets(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS);

            Mesh->BindDraw(VkContext->GetCommandList());
        }
    }
    
    RenderTarget->EndRenderPass(VkContext->GetCommandList());
    
    //Second Pass
    {
        VkClearValue clearValues[1];
        clearValues[0].color        =
        {
            { 0.2f, 0.2f, 0.2f, 1.0f }
        };

        FrameBuffer->BeginPass(VkContext->GetCurrtIndex(),clearValues,VkContext->GetCommandList());
        
        vkCmdSetViewport(VkContext->GetCommandList(), 0, 1, &viewport);
        vkCmdSetScissor(VkContext->GetCommandList(),  0, 1, &scissor);

        {
            vkCmdBindPipeline(VkContext->GetCommandList(), VK_PIPELINE_BIND_POINT_GRAPHICS, mFilterMaterial->mPipeline->Pipeline);
            mFilterMaterial->SetTexture("InputImageTexture", ColorRT);
            mFilterMaterial->BindDescriptorSets(VkContext->GetCommandList(), VK_PIPELINE_BIND_POINT_GRAPHICS);
            mQuad->Meshes[0]->BindDraw(VkContext->GetCommandList());
        }

        vkCmdEndRenderPass(VkContext->GetCommandList());
    }
}

void AnimationTextureLayer::OnUIRender(Timestep ts)
{
    ImGui::SetNextWindowPos(ImVec2(0, 0)); 
    ImGui::Begin("InputAttachmentsDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::Text("Color Depth");
    
    
    ImGui::SliderFloat("Time", &m_AnimTime, 0.0f, m_AnimDuration);
    ImGui::SliderFloat("Z-Near", &DebugParam.zNear, 0.1f, 3000.0f);
    ImGui::SliderFloat("Z-Far", &DebugParam.zFar, 0.1f, 6000.0f);

    ImGui::End();
}

void AnimationTextureLayer::OnChangeWindowSize(std::shared_ptr<ReEngine::Event> e)
{
    
}


void AnimationTextureLayer::CreateRenderTarget()
{
    auto device = VkContext->Instance->GetDevice();
    
    ColorRT = VulkanTexture::CreateRenderTarget(
        device,
        PixelFormatToVkFormat(VkContext->Instance->GetPixelFormat(), false),
        VK_IMAGE_ASPECT_COLOR_BIT,
        FrameBuffer->m_Width,
        FrameBuffer->m_Height,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
    );

    DepthRT = VulkanTexture::CreateRenderTarget(
        device,
        PixelFormatToVkFormat(PF_DepthStencil, false),
        VK_IMAGE_ASPECT_DEPTH_BIT,
        FrameBuffer->m_Width,
        FrameBuffer->m_Height,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
    );

    VulkanRenderPassInfo PassInfo 
    {
        ColorRT,VK_ATTACHMENT_LOAD_OP_CLEAR,VK_ATTACHMENT_STORE_OP_NONE,
        DepthRT,VK_ATTACHMENT_LOAD_OP_CLEAR,VK_ATTACHMENT_STORE_OP_STORE
    };

    RenderTarget = VulkanRenderTarget::Create( 
        device,
        PassInfo
    );
}

void AnimationTextureLayer::LoadAsset()
{
    auto device = VkContext->Instance->GetDevice();
    
    m_RingBuffer = CreateRef<VulkanDynamicBufferRing>();
    m_RingBuffer->OnCreate(VkContext->Instance->GetDevice(),3,200 * 1024 * 1024);

    {
        SceneShader = VulkanShader::Create(device,true,&ANIMATIONTEXTURE_VERT,&ANIMOBJ_FRAG,nullptr,nullptr,nullptr,nullptr); 
        SceneMaterial = VulkanMaterial::Create(
            device,
            RenderTarget->GetRenderPass(),
            VkContext->CommandPool->m_PipelineCache,
            SceneShader,
            m_RingBuffer
        );
        SceneMaterial->PreparePipeline();
    }
    
    auto cmdBuffer = VulkanCommandBuffer::Create(device, VkContext->CommandPool->m_CommandPool);

    // scene model
    SceneModel = VulkanModel::LoadFromFile(
        "Assets/Mesh/xiaonan/nvhai.FBX",
        device,
        cmdBuffer,
        SceneShader->perVertexAttributes
    );
    
    std::vector<std::string> diffusePaths = {
        "Assets/Mesh/xiaonan/b001.jpg"
    };
    
    TextureArray.resize(diffusePaths.size());
    for(int32 index = 0 ; index < diffusePaths.size();index++)
    {
        TextureArray[index] = VulkanTexture::Create2D(
            diffusePaths[index],
            device,
            cmdBuffer
        );
    }

    {
        mFilterShader = VulkanShader::Create(device,true,&QUAD_VERT,&COLORFILTER_FRAG,nullptr,nullptr,nullptr,nullptr);
        mFilterMaterial = VulkanMaterial::Create(
            device,
            FrameBuffer->m_RenderPass,
            VkContext->CommandPool->m_PipelineCache,
            mFilterShader,
            m_RingBuffer
        );
        mFilterMaterial->PreparePipeline();
    }
    
    SceneModel->SetAnimation(0);
    m_AnimDuration = SceneModel->Animations[0].Duration;
    m_AnimTime = 0.0f;
    
    AnimTexture = SceneModel->GenerateAnimationTexture(0);
    
    // quad model
    Quad FilterQuad;

    mQuad = VulkanModel::Create(
        device,
        cmdBuffer,
        FilterQuad.GetVertexs(),
        FilterQuad.GetIndices(),
        mFilterShader->perVertexAttributes
    );

    BoundingBox Bounds = SceneModel->RootNode->GetBounds();
    glm::vec3 BoundsSize = Bounds.Max - Bounds.Min;
    glm::vec3 BoundsCenter = Bounds.Min + BoundsSize * 0.5f;

    m_Camera = CreateRef<EditorCamera>();
    m_Camera->SetCenter(glm::vec3(BoundsCenter.x,BoundsCenter.y,BoundsCenter.z - BoundsSize.length() * 2.0));
    
    m_MVPData.model = glm::identity<glm::mat4>();
    m_MVPData.projection = glm::identity<glm::mat4>();
    m_MVPData.view = m_Camera->GetViewMatrix();
}
