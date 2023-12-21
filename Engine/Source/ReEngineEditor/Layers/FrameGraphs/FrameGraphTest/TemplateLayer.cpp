#include "TemplateLayer.h"

#include <ColorFilter_frag.h>
#include <quad_vert.h>
#include "Mesh/Quad.h"
#include "Platform/Vulkan/VulkanContext.h"

void FrameGraphTemplateLayer::OnCreateBackBuffer()
{
    GraphicalLayer::OnCreateBackBuffer();
}

void FrameGraphTemplateLayer::OnInit()
{
    mFrameGraphBuilder.Init(VkContext);
    mFrameGraph.Init(&mFrameGraphBuilder);
    mFrameGraph.Parse("Assets/Graphs/TestFrameGraph.json");
    mFrameGraph.Compile();
    
    LoadAsset();
}

void FrameGraphTemplateLayer::OnDeInit()
{
    m_Camera.reset();

    mQuad.reset();
    mFilterShader.reset();
    mFilterMaterial.reset();

    /*TODO Clear the Resources*/

    mFrameGraph.ShutDown();
    mFrameGraphBuilder.ShutDown();
    m_RingBuffer.reset();
}

void FrameGraphTemplateLayer::OnUpdate(Timestep ts)
{
    m_RingBuffer->OnBeginFrame();
    m_Camera->OnUpdate(ts);

    //TODO Write your logic

    m_MVPData.view = m_Camera->GetViewMatrix();
    m_MVPData.projection = m_Camera->GetProjection();
}

void FrameGraphTemplateLayer::OnRender()
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
    
    
    /*
     * TODO Write your Logic
     */
    

    
    // FullScreen Pass
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
            
            mFilterMaterial->BindDescriptorSets(VkContext->GetCommandList(), VK_PIPELINE_BIND_POINT_GRAPHICS);
            mQuad->Meshes[0]->BindDraw(VkContext->GetCommandList());
        }

        vkCmdEndRenderPass(VkContext->GetCommandList());
    }
}

void FrameGraphTemplateLayer::OnUIRender(Timestep ts)
{
    /*
     * TODO Write your logic
     */
}

void FrameGraphTemplateLayer::OnChangeWindowSize(std::shared_ptr<ReEngine::Event> e)
{
    GraphicalLayer::OnChangeWindowSize(e);
}

void FrameGraphTemplateLayer::LoadAsset()
{
    auto device = VkContext->Instance->GetDevice();
    
    m_RingBuffer = CreateRef<VulkanDynamicBufferRing>();
    m_RingBuffer->OnCreate(VkContext->Instance->GetDevice(),3,200 * 1024 * 1024);

    auto cmdBuffer = VulkanCommandBuffer::Create(device, VkContext->CommandPool->m_CommandPool);


    /*
     * TODO Write your logic
     * Remember to bind RT's Renderpass for your material not FrameBuffer's renderpass!
     */



    
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
