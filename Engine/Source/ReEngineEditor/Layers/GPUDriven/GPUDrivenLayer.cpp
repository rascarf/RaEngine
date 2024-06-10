#include "GPUDrivenLayer.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/Component/StaticMeshComponent.h"
#include "Platform/Vulkan/RenderGraph/Passes/PreDepthPass.h"
#include "Platform/Vulkan/RenderGraph/Passes/SimpleDiffusePass.h"

void GPUDrivenLayer::OnNativeLoadAsset()
{
    m_Camera->SetSpeed(10.0f);
    const auto device = VkContext->Instance->GetDevice();
    auto cmdBuffer = VulkanCommandBuffer::Create(device, VkContext->CommandPool->m_CommandPool);
    
    m_Scene->ReqeustParseFrameGraph("Assets/Graphs/TestFrameGraph.json");
    m_Scene->m_FrameGraph->Compile();
    
    Ref<PreDepthRenderPass> DepthTemplateRenderPass = PreDepthRenderPass::Create();
    DepthTemplateRenderPass->Init(m_RingBuffer);

    Ref<SimpleDiffusePass> DiffuseRenderPass = SimpleDiffusePass::Create();
    DiffuseRenderPass->Init(m_RingBuffer);
    
    m_Scene->RegisterPass("depth_pre_pass",DepthTemplateRenderPass);
    m_Scene->RegisterPass("shading_pass",DiffuseRenderPass);
    
    ModelLoadRequest Request;
    Request.filename = "Assets/Mesh/Room/miniHouse_FBX.FBX";
    m_Scene->ReqeustLoadModel(Request);
    m_Scene->HandleLoadList(device,cmdBuffer);
}

void GPUDrivenLayer::OnNativeUpdate(Timestep ts)
{
    m_Scene->Update(ts);
}

void GPUDrivenLayer::OnNativeDeInit()
{
    m_Scene.reset();
}

void GPUDrivenLayer::OnRender()
{
    VkCommandBufferBeginInfo cmdBeginInfo;
    ZeroVulkanStruct(cmdBeginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
    VERIFYVULKANRESULT(vkBeginCommandBuffer(VkContext->GetCommandList(), &cmdBeginInfo));
    
    m_Scene->m_FrameGraph->Render(VkContext->GetCommandList(),m_Scene.get());
    
    //Second Pass
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
            mFilterMaterial->SetTexture("InputImageTexture",  m_Scene->m_FrameGraph->GetResource("gbuffer_colour")->ResourceInfo.Texture.Texture);
            mFilterMaterial->BindDescriptorSets(VkContext->GetCommandList(), VK_PIPELINE_BIND_POINT_GRAPHICS);
            mQuad->Meshes[0]->BindDraw(VkContext->GetCommandList());
        }

        vkCmdEndRenderPass(VkContext->GetCommandList());
    }
}
