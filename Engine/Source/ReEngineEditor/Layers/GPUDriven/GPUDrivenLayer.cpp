#include "GPUDrivenLayer.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/Component/StaticMeshComponent.h"
#include "Platform/Vulkan/RenderGraph/Passes/PreDepthPass.h"

void GPUDrivenLayer::OnNativeLoadAsset()
{
    const auto device = VkContext->Instance->GetDevice();
    auto cmdBuffer = VulkanCommandBuffer::Create(device, VkContext->CommandPool->m_CommandPool);
    
    m_Scene->ReqeustParseFrameGraph("Assets/Graphs/TestFrameGraph.json");
    
    Ref<PreDepthRenderPass> DepthTemplateRenderPass = PreDepthRenderPass::Create();
    DepthTemplateRenderPass->Init(m_RingBuffer);
    
    m_Scene->m_FrameGraph->GetNode("gbuffer_pass")->Enabled = false;
    m_Scene->m_FrameGraph->Compile();
    
    m_Scene->RegisterPass("depth_pre_pass",DepthTemplateRenderPass);
    
    ModelLoadRequest Request;
    Request.filename = "Assets/Mesh/Room/miniHouse_FBX.FBX";
    
    m_Scene->ReqeustLoadModel(Request);

    m_Scene->HandleLoadList(device,cmdBuffer);

    for(auto& Entity : m_Scene->EntityNodes)
    {
        if(Entity->HasComponent<StaticMeshComponent>())
        {
            RE_INFO("{0}",Entity->GetComponent<StaticMeshComponent>().PbrMaterialParamIndex);
        }
    }
}

void GPUDrivenLayer::OnNativeUpdate(Timestep ts)
{
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

    m_Scene->m_FrameGraph->Render(VkContext->GetCommandList(),m_Scene.get());
    
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
