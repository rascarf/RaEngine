#include "TileBasedForwardLayer.h"

#include <ColorFilter_frag.h>
#include <quad_vert.h>

#include <FPObj_vert.h>
#include <FPObj_frag.h>

#include <PreDepth_vert.h>
#include <PreDepth_frag.h>

#include <LightCulling_comp.h>

#include "Math/Math.h"
#include "Platform/Vulkan/VulkanContext.h"

#include "Mesh/Quad.h"

void TileBasedForwardLayer::OnCreateBackBuffer()
{
    GraphicalLayer::OnCreateBackBuffer();
}

void TileBasedForwardLayer::OnInit()
{
    CreateRenderTarget();
    LoadAsset();
}

void TileBasedForwardLayer::OnDeInit()
{
    m_Camera.reset();
    ColorRT.reset();
    DepthRT.reset();
    RenderTarget.reset();

    mQuad.reset();
    mFilterShader.reset();
    mFilterMaterial.reset();

    Model.reset();
    ModelShader.reset();
    ModelMaterial.reset();

    // PreDepth
    PreDepthShader.reset();
    PreDepthMaterial.reset();
    PreDepthTexture.reset();
    PreDepthRenderTarget.reset();

    // Compute Pass
    ComputeShader.reset();
    ComputeProcessor.reset();
    LightCullingBuffer.reset();
    
    m_RingBuffer.reset();
}

void TileBasedForwardLayer::OnUpdate(Timestep ts)
{
    m_RingBuffer->OnBeginFrame();
    
    m_Camera->OnUpdate(ts);

    //TODO Write your logic

    UpdateLights(ts);

    CullingParam.invViewProj = glm::inverse(m_Camera->GetViewMatrix());
    CullingParam.Pos = glm::vec4(m_Camera->GetPosition(),1.0f);

    m_MVPData.view = m_Camera->GetViewMatrix();
    m_MVPData.projection = m_Camera->GetProjection();
}

void TileBasedForwardLayer::OnRender()
{
    VkCommandBufferBeginInfo cmdBeginInfo;
    ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
    VERIFYVULKANRESULT(vkBeginCommandBuffer(VkContext->GetCommandList(), &cmdBeginInfo));

    //PreDepthPass
    {
        PreDepthRenderTarget->BeginRenderPass(VkContext->GetCommandList());
        for(int32 i = 0 ; i < Model->Meshes.size() ;++i)
        {
            vkCmdBindPipeline(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS,PreDepthMaterial->mPipeline->Pipeline);

            m_MVPData.model = Model->Meshes[i]->LinkNode.lock()->GetGlobalMatrix();
            m_MVPData.view = m_Camera->GetViewMatrix();
            m_MVPData.projection = m_Camera->GetProjection();

            PreDepthMaterial->SetLocalUniform("uboMVP",&m_MVPData,sizeof(MVPBlock));
            PreDepthMaterial->BindDescriptorSets(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS);

            Model->Meshes[i]->BindDraw(VkContext->GetCommandList());
        }
        
        PreDepthRenderTarget->EndRenderPass(VkContext->GetCommandList());
    }
    
    //ComputePass
    {
        VkBufferMemoryBarrier bufferBarrier;
        ZeroVulkanStruct(bufferBarrier, VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER);
        bufferBarrier.buffer = LightCullingBuffer->Buffer;
        bufferBarrier.size   = LightCullingBuffer->Size;
        bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        
        bufferBarrier.srcQueueFamilyIndex = VkContext->GetVulkanInstance()->GetDevice()->GetGraphicsQueue()->GetFamilyIndex();
        bufferBarrier.dstQueueFamilyIndex = VkContext->GetVulkanInstance()->GetDevice()->GetComputeQueue()->GetFamilyIndex();

        vkCmdPipelineBarrier(
            VkContext->GetCommandList(),
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0,
            0,
            nullptr,
            1,
            &bufferBarrier,
            0,
            nullptr
        );

        ComputeProcessor->SetUniform("uboCulling", &CullingParam,sizeof(CullingParamBlock));
        ComputeProcessor->SetUniform("uboLights", &LightParam,sizeof(LightsParamBlock));
        ComputeProcessor->SetTexture("depthTexture",PreDepthTexture);
        
        ComputeProcessor->BindDispatch(VkContext->GetCommandList(), TileCountPerRow, TileCountPerColumn, 1);

        bufferBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        bufferBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        bufferBarrier.srcQueueFamilyIndex = VkContext->GetVulkanInstance()->GetDevice()->GetComputeQueue()->GetFamilyIndex();
        bufferBarrier.dstQueueFamilyIndex = VkContext->GetVulkanInstance()->GetDevice()->GetGraphicsQueue()->GetFamilyIndex();

        vkCmdPipelineBarrier(
            VkContext->GetCommandList(),
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0,
            nullptr,
            1,
            &bufferBarrier,
            0,
            nullptr
        );
    }
    
    // Obj Pass
    {
        RenderTarget->BeginRenderPass(VkContext->GetCommandList());
    
        for (int32 i = 0 ; i < Model->Meshes.size();++i)
        {
            vkCmdBindPipeline(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS,ModelMaterial->mPipeline->Pipeline);

            m_MVPData.model = Model->Meshes[i]->LinkNode.lock()->GlobalMatrix;

            ModelMaterial->SetLocalUniform("uboMVP",&m_MVPData,sizeof(MVPBlock));
            ModelMaterial->SetLocalUniform("uboLights",&LightParam,sizeof(LightsParamBlock));
            ModelMaterial->SetLocalUniform("uboCulling",&CullingParam,sizeof(CullingParamBlock));
            ModelMaterial->SetLocalUniform("uboDebug",&Debug,sizeof(glm::vec4));

            ModelMaterial->BindDescriptorSets(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS);
            Model->Meshes[i]->BindDraw(VkContext->GetCommandList());
        }
    
        RenderTarget->EndRenderPass(VkContext->GetCommandList());
    }
    
    //Filter Pass
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

void TileBasedForwardLayer::OnUIRender(Timestep ts)
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    // ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
    ImGui::Begin("TileBasedForwardRenderingDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    int index = Debug.x;
    ImGui::Combo("Debug", &index, "None\0Normal\0\Tile\0");
    Debug.x = index;

    // ImGui::Text("%.3f ms/frame (%d FPS)", 1000.0f / m_LastFPS, m_LastFPS);
    ImGui::End();
}

void TileBasedForwardLayer::OnChangeWindowSize(std::shared_ptr<ReEngine::Event> e)
{
    GraphicalLayer::OnChangeWindowSize(e);
}

void TileBasedForwardLayer::CreateRenderTarget()
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

void TileBasedForwardLayer::LoadAsset()
{
    auto device = VkContext->Instance->GetDevice();
    
    m_RingBuffer = CreateRef<VulkanDynamicBufferRing>();
    m_RingBuffer->OnCreate(VkContext->Instance->GetDevice(),3,200 * 1024 * 1024);

    auto cmdBuffer = VulkanCommandBuffer::Create(device, VkContext->CommandPool->m_CommandPool);
    
    /*
     * Remember to bind RT's Renderpass for your material not FrameBuffer's renderpass!
     */

    Model = VulkanModel::LoadFromFile(
        "Assets/Mesh/scene1.obj",
        device,
        cmdBuffer,
        {
            VertexAttribute::VA_Position,
            VertexAttribute::VA_UV0,
            VertexAttribute::VA_Normal,
        }
    );

    ModelShader = VulkanShader::Create(
        device,
        true,
        &FPOBJ_VERT,
        &FPOBJ_FRAG,
        nullptr,nullptr,nullptr,nullptr);

    ModelMaterial = VulkanMaterial::Create(
        device,
        RenderTarget->GetRenderPass(),
        VkContext->CommandPool->m_PipelineCache,
        ModelShader,
        m_RingBuffer
    );
    ModelMaterial->mPipelineInfo.RasterizationState.cullMode = VK_CULL_MODE_NONE;
    ModelMaterial->PreparePipeline();
    
    InitLightParams();

    PreDepthTexture = VulkanTexture::CreateRenderTarget(
        device,
        PixelFormatToVkFormat(PF_DepthStencil,false),
        VK_IMAGE_ASPECT_DEPTH_BIT,
        VkContext->Instance->GetWidth(),
        VkContext->Instance->GetHeight(),
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
    );
    
    VulkanRenderPassInfo PassInfo (PreDepthTexture,VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE);
    PreDepthRenderTarget = VulkanRenderTarget::Create(device,PassInfo);
    PreDepthShader = VulkanShader::Create(
        device,
        true,
        &PREDEPTH_VERT,
        &PREDEPTH_FRAG,
        nullptr,nullptr,nullptr,nullptr
    );

    PreDepthMaterial = VulkanMaterial::Create(
        device,
        PreDepthRenderTarget->GetRenderPass(),
        VkContext->CommandPool->m_PipelineCache,
        PreDepthShader,
        m_RingBuffer
    );
    PreDepthMaterial->mPipelineInfo.RasterizationState.cullMode = VK_CULL_MODE_NONE;
    PreDepthMaterial->mPipelineInfo.ColorAttachmentsCount = 0;
    PreDepthMaterial->PreparePipeline();
    
    LightCullingBuffer = VulkanBuffer::CreateBuffer(
        device,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        sizeof(LightVisiblity) * TileCountPerColumn * TileCountPerRow
    );
    
    ComputeShader = VulkanShader::CreateCompute(device,true,&LIGHTCULLING_COMP);
    ComputeProcessor = VulkanComputeMaterial::Create(
        device,
        VkContext->CommandPool->m_PipelineCache,
        ComputeShader,
        m_RingBuffer 
    );

    ComputeProcessor->SetStorageBuffer("lightsCullingBuffer",LightCullingBuffer);
    ModelMaterial->SetStorageBuffer("lightsCullingBuffer",LightCullingBuffer);
    
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

    BoundingBox Bounds = Model->RootNode->GetBounds();
    glm::vec3 BoundsSize = Bounds.Max - Bounds.Min;
    glm::vec3 BoundsCenter = Bounds.Min + BoundsSize * 0.5f;
    
    m_Camera = CreateRef<EditorCamera>();
    m_Camera->SetSpeed(0.1f);
    m_Camera->SetCenter(glm::vec3(BoundsCenter.x,BoundsCenter.y,BoundsCenter.z - BoundsSize.length() * 2.0));
    
    m_MVPData.model = glm::identity<glm::mat4>();
    m_MVPData.projection = glm::identity<glm::mat4>();
    m_MVPData.view = m_Camera->GetViewMatrix();
}

void TileBasedForwardLayer::InitLightParams()
{
    LightParam.Count = glm::vec4(LIGHT_SIZE,LIGHT_SIZE,LIGHT_SIZE,LIGHT_SIZE);
    BoundingBox Bounds = Model->RootNode->GetBounds();

    for(int i = 0 ; i < LIGHT_SIZE ; i++)
    {
        glm::vec3 position;
        position.x = Math::FRandRange(Bounds.Min.x, Bounds.Max.x);
        position.y = Math::FRandRange(Bounds.Min.y, Bounds.Max.y);
        position.z = Math::FRandRange(Bounds.Min.z, Bounds.Max.z);

        float radius = Math::FRandRange(0.5f, 2.5f);

        glm::vec3 color;
        color.x = Math::FRandRange(0.0f, 2.5f);
        color.y = Math::FRandRange(0.0f, 2.5f);
        color.z = Math::FRandRange(0.0f, 2.5f);

        PointLight light(position, radius, color);

        LightParam.Lights[i] = light;

        mLightInfo.Direction[i] = glm::vec3(Math::FRandRange(-1.0f, 1.0f),Math::FRandRange(-1.0f, 1.0f),Math::FRandRange(-1.0f, 1.0f));
        
        mLightInfo.Direction[i] = glm::normalize(mLightInfo.Direction[i]);
        mLightInfo.Position[i] = position;
    }

    TileCountPerRow = (VkContext->Instance->GetWidth() - 1) / TILE_SIZE + 1;
    TileCountPerColumn = (VkContext->Instance->GetHeight() - 1) / TILE_SIZE + 1;
    
    CullingParam.FrameSize.x = VkContext->Instance->GetWidth();
    CullingParam.FrameSize.y = VkContext->Instance->GetHeight();
    CullingParam.TileNum.x = TileCountPerRow;
    CullingParam.TileNum.y = TileCountPerColumn;
}

void TileBasedForwardLayer::UpdateLights(Timestep ts)
{
    BoundingBox Bound = Model->RootNode->GetBounds();
    glm::vec3 Extend = Bound.Max - Bound.Min;
    float size = Math::Min(Extend.x,Math::Min(Extend.y,Extend.z));

    for(int32 i = 0 ; i < LightParam.Count.x;++i)
    {
        PointLight& light = LightParam.Lights[i];

        light.Position = mLightInfo.Position[i] + mLightInfo.Direction[i] * Math::Cos(ts.GetSeconds()) * size;
    }
}