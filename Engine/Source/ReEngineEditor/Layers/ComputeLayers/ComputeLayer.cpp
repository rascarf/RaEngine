#include "ComputeLayer.h"

#include <ColorFilter_frag.h>
#include <quad_vert.h>

#include "Platform/Vulkan/VulkanContext.h"
#include "Texture_vert.h"
#include "Texture_frag.h"
#include "Gamma_comp.h"
#include "Contrast_comp.h"
#include "ColorInvert_comp.h"
#include "Mesh/Quad.h"

void ComputeLayer::OnCreateBackBuffer()
{
    GraphicalLayer::OnCreateBackBuffer();
}

void ComputeLayer::OnInit()
{
    CreateRenderTarget();
    LoadAsset();
    ProcessImage();
}

void ComputeLayer::OnDeInit()
{
    m_Camera.reset();
    ColorRT.reset();
    DepthRT.reset();
    RenderTarget.reset();

    mQuad.reset();
    mFilterShader.reset();
    mFilterMaterial.reset();
    
    PlaneModel.reset();
    mMaterial.reset();
    mShader.reset();
    mTexture.reset();

    for (int32 i = 0; i < 3; ++i)
    {
        ComputeTargets[i].reset();
        ComputeShaders[i].reset();
        ComputeMaterial[i].reset();
    }
    
    m_RingBuffer.reset();
}

void ComputeLayer::OnUpdate(Timestep ts)
{
    m_RingBuffer->OnBeginFrame();
    m_Camera->OnUpdate(ts);

    // m_Camera->SetFarPlane(DebugParam.zFar);
    // m_Camera->SetNearPlane(DebugParam.zNear);

    m_MVPData.view = m_Camera->GetViewMatrix();
    m_MVPData.projection = m_Camera->GetProjection();
}

void ComputeLayer::OnRender()
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
    
    vkCmdBindPipeline(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS,mMaterial->mPipeline->Pipeline);
    for(int32 i = 0 ; i < PlaneModel->Meshes.size(); ++i)
    {
        m_MVPData.model = PlaneModel->Meshes[i]->LinkNode.lock()->GetGlobalMatrix();
        
        mMaterial->SetLocalUniform("uboMVP",&m_MVPData,sizeof(MVPBlock));
        
        mMaterial->BindDescriptorSets(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS);
        PlaneModel->Meshes[i]->BindDraw(VkContext->GetCommandList());
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

void ComputeLayer::OnUIRender(Timestep ts)
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("OptimizeComputeShaderDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    if (ImGui::Combo("Filter", &m_FilterIndex, m_FilterNames.data(), m_FilterNames.size()))
    {
        if (m_FilterIndex == 0)
        {
            mMaterial->SetTexture("DiffuseMap", mTexture);
        }
        else
        {
            mMaterial->SetTexture("DiffuseMap", ComputeTargets[m_FilterIndex - 1]);
        }
    }
    
    ImGui::End();
}

void ComputeLayer::OnChangeWindowSize(std::shared_ptr<ReEngine::Event> e)
{
   
}

void ComputeLayer::CreateRenderTarget()
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

void ComputeLayer::LoadAsset()
{
    auto device = VkContext->Instance->GetDevice();
    
    m_RingBuffer = CreateRef<VulkanDynamicBufferRing>();
    m_RingBuffer->OnCreate(VkContext->Instance->GetDevice(),3,200 * 1024 * 1024);

    auto cmdBuffer = VulkanCommandBuffer::Create(device, VkContext->CommandPool->m_CommandPool);

    mTexture = VulkanTexture::Create2D(
        "Assets/Textures/game0.jpg",
        device,
        cmdBuffer,
        VK_IMAGE_USAGE_SAMPLED_BIT  | VK_IMAGE_USAGE_TRANSFER_DST_BIT  |VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
        ImageLayoutBarrier::ComputeGeneralRW
    );

    mShader = VulkanShader::Create(
        device,
        true,
        &TEXTURE_VERT,
        &TEXTURE_FRAG,
        nullptr,nullptr,nullptr,nullptr
    );
    
    mMaterial = VulkanMaterial::Create(
        device,
        RenderTarget->GetRenderPass(),
        VkContext->CommandPool->m_PipelineCache,
        mShader,
        m_RingBuffer
    );
    mMaterial->mPipelineInfo.RasterizationState.cullMode = VK_CULL_MODE_NONE;
    mMaterial->PreparePipeline(); 
    mMaterial->SetTexture("DiffuseMap",mTexture);

    PlaneModel = VulkanModel::LoadFromFile(
        "Assets/Mesh/plane_z.obj",
        device,
        cmdBuffer,
        mShader->perVertexAttributes
    );

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

        // quad model
        Quad FilterQuad;

        mQuad = VulkanModel::Create(
            device,
            cmdBuffer,
            FilterQuad.GetVertexs(),
            FilterQuad.GetIndices(),
            mFilterShader->perVertexAttributes
        );
    }

    BoundingBox Bounds = PlaneModel->RootNode->GetBounds();
    glm::vec3 BoundsSize = Bounds.Max - Bounds.Min;
    glm::vec3 BoundsCenter = Bounds.Min + BoundsSize * 0.5f;

    m_Camera = CreateRef<EditorCamera>();
    m_Camera->SetCenter(glm::vec3(BoundsCenter.x,BoundsCenter.y,BoundsCenter.z - BoundsSize.length() * 2.0));
    m_Camera->SetSpeed(0.1f);
    
    m_MVPData.model = glm::identity<glm::mat4>();
    m_MVPData.projection = glm::identity<glm::mat4>();
    m_MVPData.view = m_Camera->GetViewMatrix();
}

void ComputeLayer::ProcessImage()
{
    auto device = VkContext->Instance->GetDevice();
    
    auto CmdBuffer = VulkanCommandBuffer::Create(
        device,
        VkContext->CommandPool->m_ComputeCommandPool,
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        device->GetComputeQueue()
    );

    ComputeTargets.resize(3);
    ComputeShaders.resize(3);
    ComputeMaterial.resize(3);
    
    for(int32 i = 0 ; i < 3 ; i++)
    {
        ComputeTargets[i] = VulkanTexture::Create2D(
            device,
            CmdBuffer,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_ASPECT_COLOR_BIT,
            mTexture->Width,
            mTexture->Height,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
            VK_SAMPLE_COUNT_1_BIT,
            ImageLayoutBarrier::ComputeGeneralRW 
        );
    }
    
    ComputeShaders[0] = VulkanShader::Create(device,true,nullptr,nullptr,nullptr,&CONTRAST_COMP,nullptr,nullptr);
    ComputeShaders[1] = VulkanShader::Create(device,true,nullptr,nullptr,nullptr,&GAMMA_COMP,nullptr,nullptr);
    ComputeShaders[2] = VulkanShader::Create(device,true,nullptr,nullptr,nullptr,&COLORINVERT_COMP,nullptr,nullptr);

    for(int i = 0 ; i < 3 ; ++i)
    {
        ComputeMaterial[i] = VulkanComputeMaterial::Create(device,VkContext->CommandPool->m_PipelineCache,ComputeShaders[i],m_RingBuffer);
        ComputeMaterial[i]->SetStorageTexture("inputImage",mTexture);
        ComputeMaterial[i]->SetStorageTexture("outputImage",ComputeTargets[i]);
    }

    CmdBuffer->Begin();
    for(int32 i = 0 ; i < 3 ; ++i)
    {
        ComputeMaterial[i]->BindDispatch(CmdBuffer->CmdBuffer,ComputeTargets[i]->Width / 16, ComputeTargets[i]->Height / 16, 1);
    }
    CmdBuffer->End();
    CmdBuffer->Submit();

    m_FilterIndex = 0;
    m_FilterNames.resize(4);
    m_FilterNames[0] = "Original";
    m_FilterNames[1] = "Contrast";
    m_FilterNames[2] = "Gamma";
    m_FilterNames[3] = "ColorInvert";
}
