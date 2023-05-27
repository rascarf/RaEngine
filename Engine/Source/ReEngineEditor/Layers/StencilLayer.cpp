#include "StencilLayer.h"

#include <obj_frag.h>
#include <obj_vert.h>
#include <quad_vert.h>
#include <FilterPixelation_frag.h>
#include <ColorFilter_frag.h>

#include "Mesh/Quad.h"
#include "Platform/Vulkan/VulkanContext.h"

void RTLayer::OnCreateBackBuffer()
{
    GraphicalLayer::OnCreateBackBuffer();
}

void RTLayer::OnInit()
{
    CreateRenderTarget();
    LoadAsset();
}

void RTLayer::OnDeInit()
{
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

    RenderTarget.reset();
    SceneShader.reset();
    mFilterShader.reset();
    
    SceneMaterial.reset();
    mFilterMaterial.reset();
    
    m_RingBuffer.reset();
}

void RTLayer::OnUpdate(Timestep ts)
{
    m_RingBuffer->OnBeginFrame();
    m_Camera->OnUpdate(ts);
    
    SceneModel->RootNode->LocalMatrix = glm::rotate(SceneModel->RootNode->LocalMatrix,ts.GetSeconds() *  glm::radians(45.0f),glm::vec3(0.0f, 1.0f, 0.0f));

    m_Camera->SetFarPlane(DebugParam.zFar);
    m_Camera->SetNearPlane(DebugParam.zNear);
    
    m_VPData.view = m_Camera->GetViewMatrix();
    m_VPData.projection = m_Camera->GetProjection();
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

    const auto ViewBufferView = m_RingBuffer->AllocConstantBuffer(sizeof(ViewProjectionBlock),&m_VPData);
    {
        RenderTarget->BeginRenderPass(VkContext->GetCommandList());
        
        vkCmdBindPipeline(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS,SceneMaterial->mPipeline->Pipeline);
        for(int32 i = 0; i < RenderObject.size(); i++)
        {
            SceneMaterial->SetTexture("DiffuseMap",TextureArray[i]);
            for(auto& Mesh : RenderObject[i])
            {
                m_ModelData.model = Mesh->LinkNode.lock()->GetGlobalMatrix();

                SceneMaterial->SetLocalUniform("uboViewProj",ViewBufferView);
                SceneMaterial->SetLocalUniform("uboModel",&m_ModelData,sizeof(ModelBlock));
                SceneMaterial->BindDescriptorSets(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS);

                Mesh->BindDraw(VkContext->GetCommandList());
            }
        }

        RenderTarget->EndRenderPass(VkContext->GetCommandList());
    }

    //Second Pass
    {
        VkClearValue clearValues[1];
        clearValues[0].color        = {
            { 0.2f, 0.2f, 0.2f, 1.0f }
        };
        
        VkRenderPassBeginInfo renderPassBeginInfo;
        ZeroVulkanStruct(renderPassBeginInfo, VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO);
        renderPassBeginInfo.renderPass      = FrameBuffer->m_RenderPass;
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues    = clearValues;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width  = FrameBuffer->m_Width;
        renderPassBeginInfo.renderArea.extent.height = FrameBuffer->m_Height;
        renderPassBeginInfo.framebuffer = FrameBuffer->m_FrameBuffers[VkContext->GetCurrtIndex()];

        vkCmdBeginRenderPass(VkContext->GetCommandList(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); 
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

void RTLayer::LoadAsset()
{
    auto device = VkContext->Instance->GetDevice();
    
    m_RingBuffer = CreateRef<VulkanDynamicBufferRing>();
    m_RingBuffer->OnCreate(VkContext->Instance->GetDevice(),3,200 * 1024 * 1024);

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
    
    auto cmdBuffer = VulkanCommandBuffer::Create(device, VkContext->CommandPool->m_CommandPool);

    // scene model
    SceneModel = VulkanModel::LoadFromFile(
        "Assets/Mesh/Room/miniHouse_FBX.FBX",
        device,
        cmdBuffer,
        SceneShader->perVertexAttributes
    );

    std::vector<std::string> diffusePaths = {
        "Assets/Mesh/Room/miniHouse_Part1.jpg",
        "Assets/Mesh/Room/miniHouse_Part2.jpg",
        "Assets/Mesh/Room/miniHouse_Part3.jpg",
        "Assets/Mesh/Room/miniHouse_Part4.jpg"
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
    
    // quad model
    Quad FilterQuad;

    mQuad = VulkanModel::Create(
        device,
        cmdBuffer,
        FilterQuad.GetVertexs(),
        FilterQuad.GetIndices(),
        mFilterShader->perVertexAttributes
    );

    m_ModelData.model = glm::identity<glm::mat4>();
    m_VPData.projection = glm::identity<glm::mat4>();
    m_VPData.view = glm::identity<glm::mat4>();

    m_Camera = CreateRef<EditorCamera>();
}
