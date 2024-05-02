#include "StencilLayer.h"

#include <obj_frag.h>
#include <obj_vert.h>
#include <quad_vert.h>
#include <FilterPixelation_frag.h>
#include <ColorFilter_frag.h>

#include <SkyBox_vert.h>
#include <SkyBox_frag.h>
#include <Equirect2Cube_comp.h>
#include <SpecularFilter_comp.h>
#include <DiffuseFilter_comp.h>
#include <Lut_comp.h>

#include <PbrMesh_vert.h>
#include <PbrMesh_frag.h>

#include "Mesh/Quad.h"
#include "Platform/Vulkan/VulkanContext.h"

void RTLayer::OnCreateBackBuffer()
{
    GraphicalLayer::OnCreateBackBuffer();
}

void RTLayer::OnInit()
{
    VkContext->Instance->GetDevice()->mContext = VkContext;
    m_ASyncLoader = CreateRef<AsyncLoader>();
    m_ASyncLoader->Init( VkContext->Instance->GetDevice());
    
    CreateRenderTarget();
    LoadAsset();
}

void RTLayer::OnDeInit()
{
    m_ASyncLoader->ShutDown();
    m_Camera.reset();
    
    SceneModel.reset();
    mQuad.reset();

    for(auto& tex : TextureArray)
    {
        tex.reset();
    }

    for(auto& obj : RenderObject)
    {
        for(auto& mesh: obj)
        {
            mesh.reset();
        }
    }

    ColorRT.reset();
    DepthRT.reset();

    SkyBoxShader.reset();
    SkyBoxMaterial.reset();
    SkyBox.reset();
    CubeMap.reset();
    
    EnvTextureEquirect.reset();
    EnvEquirect2CubeShader.reset();       
    EnvEquirect2CubePipe.reset();
    PreFilterCubeMap.reset();

    IrraMap.reset();
    IrraPipe.reset();
    IrraShader.reset();


    for(auto& tex : PBRTextureArray)
    {
        tex.reset();
    }
    PBRModel.reset();
    PBRShader.reset();
    PBRPipeline.reset();

    SpcularFilterShader.reset();       
    SpcularFilterPipe.reset();
    EnvCubeMap.reset();

    LutShader.reset();
    LutPipe.reset();
    Lut.reset();
    
    SkyBoxRenderTarget.reset();
    RenderTarget.reset();
    
    SceneShader.reset();
    mFilterShader.reset();
    
    SceneMaterial.reset();
    mFilterMaterial.reset();
    
    m_RingBuffer.reset();
}

void RTLayer::OnUpdate(Timestep ts)
{
    m_ASyncLoader->Update_MainThread();
    
    m_RingBuffer->OnBeginFrame();
    m_Camera->OnUpdate(ts);
    
    // SceneModel->RootNode->LocalMatrix = glm::rotate(SceneModel->RootNode->LocalMatrix,ts.GetSeconds() *  glm::radians(45.0f),glm::vec3(0.0f, 1.0f, 0.0f));

    m_Camera->SetFarPlane(DebugParam.zFar);
    m_Camera->SetNearPlane(DebugParam.zNear);  
    
    m_MVPData.view = m_Camera->GetViewMatrix();
    m_MVPData.projection = m_Camera->GetProjection();
    
    LightParams.eyePosition = glm::vec4(m_Camera->GetPosition(),0.0f);
}

void RTLayer::OnRender()
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

    {
        SkyBoxRenderTarget->BeginRenderPass(VkContext->GetCommandList());
        vkCmdBindPipeline(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS,SkyBoxMaterial->mPipeline->Pipeline);

        ModelViewProjectionBlock SkyBoxBlock;
        SkyBoxBlock.model = glm::identity<glm::mat4x4>();
        SkyBoxBlock.view = m_Camera->GetViewMatrix();
        SkyBoxBlock.view[3][0] = 0.0f;
        SkyBoxBlock.view[3][1] = 0.0f;
        SkyBoxBlock.view[3][2] = 0.0f;
        
        SkyBoxBlock.projection = m_Camera->GetProjection();
        
        SkyBoxMaterial->SetLocalUniform("uboMVP",&SkyBoxBlock,sizeof(ModelViewProjectionBlock));
        SkyBoxMaterial->BindDescriptorSets(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS);

        for(const auto Mesh : SkyBox->Meshes)
        {
            Mesh->BindDraw(VkContext->GetCommandList());
        }
        
        SkyBoxRenderTarget->EndRenderPass(VkContext->GetCommandList());
    }
    
    {
        // 普通Model
        // std::vector<std::string> diffusePaths =  {
        //     "Assets/Mesh/Room/miniHouse_Part1.jpg",
        //     "Assets/Mesh/Room/miniHouse_Part2.jpg",
        //     "Assets/Mesh/Room/miniHouse_Part3.jpg",
        //     "Assets/Mesh/Room/miniHouse_Part4.jpg"
        // };
        //
        // for(int32 i = 0; i < RenderObject.size(); i++)
        // {
        //     if(TextureArray[i])
        //         SceneMaterial->SetTexture("DiffuseMap",TextureArray[i],diffusePaths[i]);
        // }
        //
        RenderTarget->BeginRenderPass(VkContext->GetCommandList());
        //
        // vkCmdBindPipeline(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS,SceneMaterial->mPipeline->Pipeline);
        // for(int32 i = 0; i < RenderObject.size(); i++)
        // {
        //     for(auto& Mesh : RenderObject[i])
        //     {
        //         m_MVPData.model = Mesh->LinkNode.lock()->GetGlobalMatrix();
        //
        //         SceneMaterial->SetLocalUniform("uboMVP",&m_MVPData,sizeof(ModelViewProjectionBlock));
        //         SceneMaterial->BindDescriptorSets(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS,diffusePaths[i]);
        //
        //         Mesh->BindDraw(VkContext->GetCommandList());
        //     }
        // }
        vkCmdBindPipeline(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS,PBRPipeline->mPipeline->Pipeline);
        for(auto& Mesh : PBRModel->Meshes)
        {
            m_MVPData.model = Mesh->LinkNode.lock()->GetGlobalMatrix();
            PBRPipeline->SetLocalUniform("uboMVP",&m_MVPData,sizeof(ModelViewProjectionBlock));
            PBRPipeline->SetLocalUniform("lightUniform",&LightParams,sizeof(ShadingUniforms));
            PBRPipeline->BindDescriptorSets(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS);
            Mesh->BindDraw(VkContext->GetCommandList());
        }
        
        RenderTarget->EndRenderPass(VkContext->GetCommandList());
    }

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
            // mFilterMaterial->SetTexture("InputImageTexture", Lut);
            mFilterMaterial->BindDescriptorSets(VkContext->GetCommandList(), VK_PIPELINE_BIND_POINT_GRAPHICS);
            mQuad->Meshes[0]->BindDraw(VkContext->GetCommandList());
        }

        vkCmdEndRenderPass(VkContext->GetCommandList());
    }
}

void RTLayer::OnUIRender(Timestep ts)
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("InputAttachmentsDemo", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::Text("Color Depth");
    
    ImGui::SliderFloat("Z-Near", &DebugParam.zNear, 0.1f, 3000.0f);
    ImGui::SliderFloat("Z-Far", &DebugParam.zFar, 0.1f, 6000.0f);

    ImGui::End();
}

void RTLayer::OnChangeWindowSize(std::shared_ptr<ReEngine::Event> e)
{
    
}

void RTLayer::CreateRenderTarget()
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

    const VulkanRenderPassInfo SkyBoxInfo
    {
        ColorRT,VK_ATTACHMENT_LOAD_OP_CLEAR,VK_ATTACHMENT_STORE_OP_NONE,nullptr
    };
    SkyBoxRenderTarget = VulkanRenderTarget::Create
    (
        device,
        SkyBoxInfo
    );

    const VulkanRenderPassInfo PassInfo
    {
        ColorRT,VK_ATTACHMENT_LOAD_OP_LOAD,VK_ATTACHMENT_STORE_OP_NONE,
        DepthRT,VK_ATTACHMENT_LOAD_OP_CLEAR,VK_ATTACHMENT_STORE_OP_STORE
    };

    RenderTarget = VulkanRenderTarget::Create(
        device,
        PassInfo
    );
}

void RTLayer::GenerateDiffuseAndSpecular()
{
    auto device = VkContext->Instance->GetDevice();
    EnvTextureEquirect = VulkanTexture::Create2DWithData(
        "Assets/Textures/environment.hdr",
        device,
        nullptr,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        ImageLayoutBarrier::ComputeGeneralRW,
        VK_SAMPLE_COUNT_1_BIT
    );
    
    const auto ComputeCmdBuffer = VulkanCommandBuffer::Create(
       device,
       VkContext->CommandPool->m_ComputeCommandPool,
       VK_COMMAND_BUFFER_LEVEL_PRIMARY,
       device->GetGraphicsQueue()
    );
    
    ComputeCmdBuffer->Begin();
    PreFilterCubeMap = VulkanTexture::CreateCube(
        device,
        ComputeCmdBuffer,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        1024,1024,
        true,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_SAMPLE_COUNT_1_BIT,
        ImageLayoutBarrier::ComputeGeneralRW
    );

    // TODO 不知道为什么SpecializationInfo用不了
    const VkSpecializationMapEntry specializationMap = { 0, 0, sizeof(uint32_t) };
    vkInfo.specializationData = (uint32_t)PreFilterCubeMap->MipLevels;
    VkSpecializationInfo Info = {1,&specializationMap,sizeof(uint32_t),&vkInfo.specializationData};
    vkInfo.Entry = specializationMap;
    vkInfo.Info = Info;
    vkInfo.VarName = "NumMipLevels";
    
    EnvEquirect2CubeShader = VulkanShader::CreateCompute(device,true,&EQUIRECT2CUBE_COMP);
    EnvEquirect2CubePipe = VulkanComputePipeline::Create(
        device,
        VkContext->CommandPool->m_PipelineCache,
        EnvEquirect2CubeShader,
        m_RingBuffer
    );

    EnvEquirect2CubePipe->SetTexture("inputTexture",EnvTextureEquirect);
    EnvEquirect2CubePipe->SetTexture("outputTexture",PreFilterCubeMap);
    
    EnvEquirect2CubePipe->BindDispatch(ComputeCmdBuffer->CmdBuffer,1024 / 32,1024 / 32 , 6);
    PreFilterCubeMap->IsUpload = true;
    
    ComputeCmdBuffer->GenerateMipMaps(PreFilterCubeMap);
    
    VulkanTexture::ImagePipelineBarrier(
        ComputeCmdBuffer->CmdBuffer,
        PreFilterCubeMap,
        PreFilterCubeMap->mImageLayoutBarrier,
        ImageLayoutBarrier::PixelGeneralRW
    );

    EnvCubeMap = VulkanTexture::CreateCube(
        device,
        ComputeCmdBuffer,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        1024,1024,
        true,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_SAMPLE_COUNT_1_BIT,
        ImageLayoutBarrier::ComputeGeneralRW
    );

    ComputeCmdBuffer->CopyImage(PreFilterCubeMap,EnvCubeMap);

    SpcularFilterShader = VulkanShader::CreateCompute(device,true,&SPECULARFILTER_COMP,vkInfo);
    
    SpcularFilterPipe = VulkanComputePipeline::Create(
        device,
        VkContext->CommandPool->m_PipelineCache,
        SpcularFilterShader,
        m_RingBuffer
    );
    SpcularFilterPipe->SetTexture("inputTexture",PreFilterCubeMap);
    
    std::vector<VkImageView> envTextureMipTailViews;
    for(int i = 1 ; i < PreFilterCubeMap->MipLevels ; i++)
    {
        envTextureMipTailViews.push_back(EnvCubeMap->CreateImageView(i,1));
    }
    SpcularFilterPipe->SetTextures("outputTexture",envTextureMipTailViews);

    const float deltaRoughness = 1.0f / std::max(float(envTextureMipTailViews.size()), 1.0f);
    for(uint32_t level = 1 ,size = EnvCubeMap->Width / 2 ; level < EnvCubeMap->MipLevels ; level++,size /= 2)
    {
        const uint32_t numGroups = std::max<uint32_t>(1, size/32);
        const SpecularFilterPushConstants pushConstants = { level-1, level * deltaRoughness };

        SpcularFilterPipe->PushContant(ComputeCmdBuffer,0,sizeof(pushConstants),(void*)&pushConstants);
        SpcularFilterPipe->BindDispatch(ComputeCmdBuffer->CmdBuffer,numGroups,numGroups,6);
    }

    IrraMap = VulkanTexture::CreateCube(device,
        ComputeCmdBuffer,
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        128,128,
        false,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_SAMPLE_COUNT_1_BIT,
        ImageLayoutBarrier::ComputeGeneralRW
    );
    IrraShader = VulkanShader::CreateCompute(device,true,&DIFFUSEFILTER_COMP);
    IrraPipe = VulkanComputePipeline::Create(
        device,
        VkContext->CommandPool->m_PipelineCache,
        IrraShader,
        m_RingBuffer
    );

    IrraPipe->SetTexture("inputTexture",EnvCubeMap);
    IrraPipe->SetTexture("outputTexture",IrraMap);
    IrraPipe->BindDispatch(ComputeCmdBuffer->CmdBuffer,128 / 32 , 128 / 32 ,6);
    
    Lut = VulkanTexture::Create2D(
        device,
        ComputeCmdBuffer,
        VK_FORMAT_R16G16_SFLOAT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        512,512,
        VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_SAMPLE_COUNT_1_BIT,
        ImageLayoutBarrier::ComputeGeneralRW
    );
    LutShader = VulkanShader::CreateCompute(device,true,&LUT_COMP);
    LutPipe = VulkanComputePipeline::Create(
        device,
        VkContext->CommandPool->m_PipelineCache,
        LutShader,
        m_RingBuffer
        );
    LutPipe->SetTexture("LUT",Lut);
    LutPipe->BindDispatch(ComputeCmdBuffer->CmdBuffer,512 / 32, 512/32,6);
    
    ComputeCmdBuffer->End();
    ComputeCmdBuffer->Submit();

    for(int i = 0 ; i < envTextureMipTailViews.size() ; i++)
    {
        vkDestroyImageView(device->GetInstanceHandle(),envTextureMipTailViews[i],nullptr);
    }
}

void RTLayer::LoadAsset()
{
    auto device = VkContext->Instance->GetDevice();
    auto cmdBuffer = VulkanCommandBuffer::Create(device, VkContext->CommandPool->m_CommandPool);
    
    m_RingBuffer = CreateRef<VulkanDynamicBufferRing>();
    m_RingBuffer->OnCreate(VkContext->Instance->GetDevice(),3,200 * 1024 * 1024);

    GenerateDiffuseAndSpecular();
    
    {
        SceneShader = VulkanShader::Create(device,true,&OBJ_VERT,&OBJ_FRAG,nullptr,nullptr,nullptr,nullptr);
        SceneMaterial = VulkanMaterial::Create(
            device,
            RenderTarget->GetRenderPass(),
            VkContext->CommandPool->m_PipelineCache,
            SceneShader,
            m_RingBuffer
        );
        
        SceneMaterial->PreparePipeline();
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
    

    // scene model
    SceneModel = VulkanModel::LoadFromFile(
        "Assets/Mesh/Room/miniHouse_FBX.FBX",
        device,
        cmdBuffer,
        SceneShader->perVertexAttributes
    );

    std::vector<std::string> diffusePaths =  {
        "Assets/Mesh/Room/miniHouse_Part1.jpg",
        "Assets/Mesh/Room/miniHouse_Part2.jpg",
        "Assets/Mesh/Room/miniHouse_Part3.jpg",
        "Assets/Mesh/Room/miniHouse_Part4.jpg"
    };
    
    TextureArray.resize(diffusePaths.size());
    for(int32 index = 0 ; index < diffusePaths.size();index++)
    {
        SceneMaterial->Prepare(diffusePaths[index]);
        
        TextureArray[index] = VulkanTexture::Create2D(
            diffusePaths[index],
            device,
            cmdBuffer
        );
        
        // m_ASyncLoader->RequestTexture(diffusePaths[index],[index,this](FileLoadCallBackParamStruct Ready)
        // {
        //     TextureArray[index] = Ready.TextureHanle;
        //     RE_INFO("Index:{3} {0} :Width:{1} Height:{2}",Ready.FilePath,Ready.TextureHanle->Width,Ready.TextureHanle->Height,index);
        // });
    }

    std::vector<std::string> FilaNames = {
        "Assets/Textures/cubemap/output_skybox_posx.hdr",
        "Assets/Textures/cubemap/output_skybox_negx.hdr",
        "Assets/Textures/cubemap/output_skybox_posy.hdr",
        "Assets/Textures/cubemap/output_skybox_negy.hdr",
        "Assets/Textures/cubemap/output_skybox_posz.hdr",
        "Assets/Textures/cubemap/output_skybox_negz.hdr"
    };
    CubeMap = VulkanTexture::CreateCube(
           FilaNames,
           device,
           cmdBuffer,
           VK_IMAGE_ASPECT_COLOR_BIT,
           VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
   );
    
    SkyBoxShader = VulkanShader::Create(device,true,&SKYBOX_VERT,&SKYBOX_FRAG,nullptr,nullptr,nullptr,nullptr);
    SkyBoxMaterial = VulkanMaterial::Create(
        device,
        SkyBoxRenderTarget->GetRenderPass(),
        VkContext->CommandPool->m_PipelineCache,
        SkyBoxShader,
        m_RingBuffer
        );
    
    SkyBoxMaterial->mPipelineInfo.RasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    SkyBoxMaterial->mPipelineInfo.DepthStencilState.depthCompareOp = VK_COMPARE_OP_ALWAYS;
    SkyBoxMaterial->mPipelineInfo.DepthStencilState.depthTestEnable = VK_FALSE;
    SkyBoxMaterial->mPipelineInfo.DepthStencilState.depthWriteEnable = VK_FALSE;
    SkyBoxMaterial->mPipelineInfo.DepthStencilState.stencilTestEnable = VK_FALSE; 
    SkyBoxMaterial->PreparePipeline();
    SkyBoxMaterial->SetTexture("diffuseMap",PreFilterCubeMap);
    
    SkyBox = VulkanModel::LoadFromFile(
        "Assets/Mesh/cube.obj",
        device,
        cmdBuffer,
        SkyBoxShader->perVertexAttributes
    );
    
    RenderObject.resize(diffusePaths.size()); 
    for(const auto& Mesh : SceneModel->Meshes) 
    {
        const std::string& DiffuseName = Mesh->Material.Diffuse;
        if (DiffuseName == "miniHouse_Part1")
        {
            RenderObject[0].push_back(Mesh);
        }
        else if (DiffuseName == "miniHouse_Part2")
        {
            RenderObject[1].push_back(Mesh);
        }
        else if (DiffuseName == "miniHouse_Part3")
        {
            RenderObject[2].push_back(Mesh);
        }
        else if (DiffuseName == "miniHouse_Part4")
        {
            RenderObject[3].push_back(Mesh);
        }
    }

    
    PBRShader = VulkanShader::Create(device,true,&PBRMESH_VERT,&PBRMESH_FRAG,nullptr,nullptr,nullptr,nullptr);
    PBRPipeline = VulkanMaterial::Create(device,RenderTarget->GetRenderPass(),VkContext->CommandPool->m_PipelineCache,PBRShader,m_RingBuffer);
    PBRPipeline->PreparePipeline();
    
    PBRModel = VulkanModel::LoadFromFile(
    "Assets/Mesh/cerberus.fbx",
        device,
        cmdBuffer,
        PBRShader->perVertexAttributes
    );

    {
        std::vector<std::string> PBRTextures{
            "Assets/Textures/cerberus_A.png",
            "Assets/Textures/cerberus_R.png",
            "Assets/Textures/cerberus_M.png",
            "Assets/Textures/cerberus_N.png",
        };
        
        PBRTextureArray.resize(4);
        PBRTextureArray[0]  = VulkanTexture::Create2DWithData(
            PBRTextures[0],
            device,
            cmdBuffer,
            VK_FORMAT_R8G8B8A8_SRGB
        );
        

        PBRTextureArray[1]  = VulkanTexture::Create2DWithData(
           PBRTextures[1],
           device,
           cmdBuffer,
           VK_FORMAT_R8_UNORM
       );

        PBRTextureArray[2]  = VulkanTexture::Create2DWithData(
           PBRTextures[2],
           device,
           cmdBuffer,
           VK_FORMAT_R8_UNORM
       );

        PBRTextureArray[3]  = VulkanTexture::Create2DWithData(
           PBRTextures[3],
           device,
           cmdBuffer,
           VK_FORMAT_R8G8B8A8_UNORM
       );
        
        PBRPipeline->SetTexture("albedoTexture",PBRTextureArray[0]);
        PBRPipeline->SetTexture("roughnessTexture",PBRTextureArray[1]);
        PBRPipeline->SetTexture("metalnessTexture",PBRTextureArray[2]);
        PBRPipeline->SetTexture("normalTexture",PBRTextureArray[3]);
        PBRPipeline->SetTexture("specularTexture",EnvCubeMap);
        PBRPipeline->SetTexture("irradianceTexture",IrraMap);
        PBRPipeline->SetTexture("specularBRDF_LUT",Lut);
    }
    
    
    // quad model
    Quad FilterQuad;

    mQuad = VulkanModel::Create(
        device,
        cmdBuffer,
        FilterQuad.GetVertexs(), 
        FilterQuad.GetIndices(),
        mFilterShader->perVertexAttributes
    );

    m_MVPData.model = glm::identity<glm::mat4>();
    m_MVPData.projection = glm::identity<glm::mat4>();
    m_MVPData.view = glm::identity<glm::mat4>();

    m_Camera = CreateRef<EditorCamera>();
    m_Camera->SetSpeed(50);

    for(int i = 0;i < NumLights ; i++)
    {
        LightParams.lights[i].radiance = glm::vec4(0.0,0.0,0.0,0.0); 
    }
}


