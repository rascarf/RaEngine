#include "SimplePathTracing.h"

#include <ColorFilter_frag.h>
#include <quad_vert.h>
#include <closethit_rchit.h>
#include <raygen_rgen.h>
#include <miss_rmiss.h>

#include "FrameWork/Scene.h"
#include "glm/gtx/io.hpp"
#include "Math/Math.h"
#include "Mesh/Quad.h"
#include "Platform/Vulkan/VulkanContext.h"

void SimplePathTracing::OnCreateBackBuffer()
{
    GraphicalLayer::OnCreateBackBuffer();
}

void SimplePathTracing::OnInit()
{
    CreateRenderTarget();
    LoadAsset();
    PrepreUniformBuffers();
    CreateMaterials();
}

void SimplePathTracing::OnDeInit()
{
    m_Camera.reset();
    
    ColorRT.reset();
    DepthRT.reset();
    RenderTarget.reset();

    mQuad.reset();
    mFilterShader.reset();
    mFilterMaterial.reset();

    m_CloseHitShader.reset();
    mLightBuffer.reset();
    mMaterialBuffer.reset();
    mObjectBuffer.reset();
    mStorageImage.reset();

    RaytracingPipeline.Destory();

    Scene.Destory();
    /*TODO Clear the Resources*/
    
    m_RingBuffer.reset();
}

void SimplePathTracing::OnUpdate(Timestep ts)
{
    m_RingBuffer->OnBeginFrame();
    m_Camera->OnUpdate(ts);

    m_MVPData.view = m_Camera->GetViewMatrix();
    m_MVPData.projection = m_Camera->GetProjection();

    m_GlobalParam.SamplingData.x = m_GlobalParam.SamplingData.x + 1;
    m_GlobalParam.SamplingData.y = Math::Atan((2.0f * Math::Tan(m_Camera->GetFOV() * 0.5f)) / 720.0f);
    m_GlobalParam.Pos = glm::vec4(m_Camera->GetPosition(),1.0f);

    m_GlobalParam.InvProj = glm::inverse(m_MVPData.projection);
    m_GlobalParam.InvView = glm::inverse(m_MVPData.view);
}

void SimplePathTracing::OnRender()
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

    // RenderTarget->BeginRenderPass(VkContext->GetCommandList());
    const auto bufferView = m_RingBuffer->AllocConstantBuffer(sizeof(GlobalParamBlock),&m_GlobalParam);
    uint32_t UniformOffset[1] = {0};
    RaytracingPipeline.RayTracingSet->WriteBuffer("globalParam",&bufferView); 
    
    vkCmdBindDescriptorSets(
        VkContext->GetCommandList(), 
        VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
        m_CloseHitShader->pipelineLayout, 0,
        1,
        &RaytracingPipeline.RayTracingSet->DescriptorSets[0],  
        1,
        UniformOffset 
    );
    
    vkCmdBindDescriptorSets(
       VkContext->GetCommandList(), 
       VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
       m_CloseHitShader->pipelineLayout, 1,
       1,
       &RaytracingPipeline.RayTracingSet->DescriptorSets[1],
       0,
       nullptr
   );

    vkCmdBindPipeline(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,RaytracingPipeline.RayTracingPipeline);
    
    auto Regions = RaytracingPipeline.ShaderBindingTableWrapper.get_regions();
    
    vkCmdTraceRaysKHR(
        VkContext->GetCommandList(),
            &Regions[0],&Regions[1],&Regions[2],&Regions[3],
            FrameBuffer->m_Width,
            FrameBuffer->m_Height,
            1
        );        
    
    // RenderTarget->EndRenderPass(VkContext->GetCommandList());
    
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
            mFilterMaterial->SetTexture("InputImageTexture", mStorageImage);
            mFilterMaterial->BindDescriptorSets(VkContext->GetCommandList(), VK_PIPELINE_BIND_POINT_GRAPHICS);
            mQuad->Meshes[0]->BindDraw(VkContext->GetCommandList());
        }
        
        vkCmdEndRenderPass(VkContext->GetCommandList());

        // ImagePipelineBarrier(VkContext->GetCommandList(), FrameBuffer->m_FrameBuffers[VkContext->GetCurrtIndex()], ImageLayoutBarrier::Undefined, ImageLayoutBarrier::DepthStencilAttachment, subResRange);
    }
}

void SimplePathTracing::OnUIRender(Timestep ts)     
{
    /*
     * TODO Write your logic  
     */
} 

void SimplePathTracing::OnChangeWindowSize(std::shared_ptr<ReEngine::Event> e) 
{
    GraphicalLayer::OnChangeWindowSize(e);
}

void SimplePathTracing::CreateRenderTarget() 
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

void SimplePathTracing::LoadAsset()
{
    auto device = VkContext->Instance->GetDevice();
    
    m_RingBuffer = CreateRef<VulkanDynamicBufferRing>();
    m_RingBuffer->OnCreate(VkContext->Instance->GetDevice(),3,200 * 1024 * 1024);
    
    /*
     * TODO Write your logic
     * Remember to bind RT's Renderpass for your material not FrameBuffer's renderpass!
     */
    
    Scene.LoadglTFModel(device,VkContext->CommandPool,"Assets/Mesh/WaterBottle.glb");
    Scene.CreateBlas();
    Scene.CreateTlas();

    auto cmdBuffer = VulkanCommandBuffer::Create(device, VkContext->CommandPool->m_CommandPool);

    mLightBuffer = VulkanBuffer::CreateBuffer(
        device,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        sizeof(LightData) * Scene.LightDatas.size(),
            Scene.LightDatas.data()
    );

    mMaterialBuffer = VulkanBuffer::CreateBuffer(
        device,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        sizeof(Material) * Scene.Materials.size(),
             Scene.Materials.data()
    );
    
    struct ObjectInstance
    {
        glm::vec<4,int32> params = glm::vec<4,int32>(-1,-1,-1,-1); // material、mesh、padding、padding
    };
    std::vector<ObjectInstance> objects(Scene.Entities.size());
    for(int32 i = 0 ; i < Scene.Entities.size(); i++)
    {
        const int MeshIndex = Scene.Entities[i]->MeshIndex;
        const auto Mesh = Scene.Meshes[MeshIndex];
        objects[i].params.x = Mesh->Material;
        objects[i].params.y = MeshIndex;
    }
    mObjectBuffer = VulkanBuffer::CreateBuffer(
        device,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
       sizeof(ObjectInstance) * objects.size(),
        objects.data()
    );

    mStorageImage = VulkanTexture::Create2D(
        device,
        cmdBuffer,
        VK_FORMAT_R32G32B32A32_SFLOAT,
        VK_IMAGE_ASPECT_COLOR_BIT,
        1280,
        720,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
        VK_SAMPLE_COUNT_1_BIT,
        ImageLayoutBarrier::ComputeGeneralRW
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

    // BoundingBox Bounds = PlaneModel->RootNode->GetBounds();
    // glm::vec3 BoundsSize = Bounds.Max - Bounds.Min;
    // glm::vec3 BoundsCenter = Bounds.Min + BoundsSize * 0.5f;
    //
    
    m_Camera = CreateRef<EditorCamera>();
    m_Camera->SetSpeed(0.1f);
    // m_Camera->SetCenter(glm::vec3(BoundsCenter.x,BoundsCenter.y,BoundsCenter.z - BoundsSize.length() * 2.0));
    
    m_MVPData.model = glm::identity<glm::mat4>();
    m_MVPData.projection = glm::identity<glm::mat4>();
    m_MVPData.view = m_Camera->GetViewMatrix();
}

void SimplePathTracing::PrepreUniformBuffers()
{
    m_GlobalParam.ViewSize.x = 1280;
    m_GlobalParam.ViewSize.y = 720;
    m_GlobalParam.ViewSize.z = 1.0f / 1280;
    m_GlobalParam.ViewSize.w = 1.0f / 720;

    m_GlobalParam.SamplingData.x = 0;
    m_GlobalParam.SamplingData.y = Math::Atan((2.0f * Math::Tan(m_Camera->GetFOV() * 0.5f)) / 720.0f);
    m_GlobalParam.SamplingData.z = 8;
    m_GlobalParam.SamplingData.w = 0.1f;

    m_GlobalParam.LightInfo.x = Scene.Lights.size();
}

void SimplePathTracing::CreateMaterials()
{
    auto device = VkContext->Instance->GetDevice();
    std::unordered_map<std::string,int32> ParamArray;
    ParamArray.insert(make_pair("indices",Scene.Meshes.size()));
    ParamArray.insert(make_pair("vertices",Scene.Meshes.size()));
    ParamArray.insert(make_pair("textures",Scene.Textures.size())); 
    
    m_CloseHitShader = VulkanShader::CreateRayTracingShader(device,&CLOSETHIT_RCHIT,&RAYGEN_RGEN,&MISS_RMISS,ParamArray);
    RaytracingPipeline.mDevice = device;
    RaytracingPipeline.CreatePipeline(m_CloseHitShader,1,std::vector<uint32_t>{},dim3{1,1,0},Scene.Tla.accel);
    RaytracingPipeline.RayTracingSet->WriteTextureArray("textures",Scene.Textures);

    std::vector<Ref<VulkanBuffer>> IndicesArray;
    std::vector<Ref<VulkanBuffer>> VertexArray;
    for(const auto& Mesh:Scene.Meshes)
    {
        IndicesArray.push_back(Mesh->IndexBuffer);
        VertexArray.push_back(Mesh->VertexBuffer);
    }
    RaytracingPipeline.RayTracingSet->WriteStorageBufferArray("indices",IndicesArray);
    RaytracingPipeline.RayTracingSet->WriteStorageBufferArray("vertices",VertexArray);

    RaytracingPipeline.RayTracingSet->WriteBuffer("lights",mLightBuffer);
    RaytracingPipeline.RayTracingSet->WriteBuffer("materials",mMaterialBuffer);
    RaytracingPipeline.RayTracingSet->WriteBuffer("objects",mObjectBuffer);
    RaytracingPipeline.RayTracingSet->WriteImage("image",mStorageImage);
}
