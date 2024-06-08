#include "PreDepthPass.h"
#include "PreDepth_frag.h"
#include "FrameWork/Component/TransformComponent.h"
#include "Platform/Vulkan/VulkanScene.h"
#include "Platform/Vulkan/Component/StaticMeshComponent.h"
#include "Renderer/RHI/Renderer.h"

namespace ReEngine
{
    inline void PreDepthRenderPass::PreRender(VkCommandBuffer Cmd, Ref<VulkanModel> Model)
    {
       
    }

    inline void PreDepthRenderPass::Render(VkCommandBuffer Cmd, Ref<VulkanModel> Model)
    {
        vkCmdBindPipeline(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS,RenderPSO->mPipeline->Pipeline);
        for(const auto& Mesh : Model->Meshes)
        {
            m_MVPData.model = Mesh->LinkNode.lock()->GetGlobalMatrix();
            RenderPSO->SetUniform("uboMVP",&m_MVPData,sizeof(ModelViewProjectionBlock));
            RenderPSO->BindDescriptorSets(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS);

            Mesh->BindDraw(VkContext->GetCommandList());
        }
    }

    void PreDepthRenderPass::PreRender(VkCommandBuffer CmdBuffer, VulkanScene* Scene)
    {
        
    }

    void PreDepthRenderPass::Render(VkCommandBuffer CmdBuffer, VulkanScene* Scene)
    {
        vkCmdBindPipeline(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS,RenderPSO->mPipeline->Pipeline);
        
        auto View =  Scene->mScene.mRegistry.view<StaticMeshComponent,TransformComponent>();

        for(auto& Entity: View)
        {
            m_MVPData.model = View.get<TransformComponent>(Entity).GetTransform();
            RenderPSO->SetUniform("uboMVP",&m_MVPData,sizeof(ModelViewProjectionBlock));
            RenderPSO->BindDescriptorSets(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS);

            if(!View.get<StaticMeshComponent>(Entity).Mesh.expired())
            {
                View.get<StaticMeshComponent>(Entity).Mesh.lock()->BindDraw(CmdBuffer);
            }
        }
    }

    void PreDepthRenderPass::PostRender(VkCommandBuffer CmdBuffer, VulkanScene* Scene)
    {
        
    }

    const std::vector<VertexAttribute>& PreDepthRenderPass::GetRenderPassAttribute()
    {
        return SceneShader->perVertexAttributes;
    }

    void PreDepthRenderPass::Update(const MVPBlock& MVPBLock)
    {
        m_MVPData.view = MVPBLock.view;
        m_MVPData.projection = MVPBLock.projection;
    }

    Ref<PreDepthRenderPass> PreDepthRenderPass::Create()
    {
        Ref<PreDepthRenderPass> Result = CreateRef<PreDepthRenderPass>();
        return Result;
    }

    void PreDepthRenderPass::Init(Ref<VulkanDynamicBufferRing> InRingBuffer)
    {
        RingBuffer = InRingBuffer;
    }

    inline void PreDepthRenderPass::Register(Ref<FrameGraphNode> Node,Ref<VulkanDevice> Device)
    {
        if(!Node->Enabled)
            return;
            
        auto Context = Renderer::GetContext().get();
        VkContext = dynamic_cast<ReEngine::VulkanContext*>(Context);
        
        SceneShader = VulkanShader::Create(Device,true,&OBJ_VERT,&PREDEPTH_FRAG,nullptr,nullptr,nullptr,nullptr);
        RenderPSO = VulkanGraphicPipeline::Create(
            Device,
            Node->RenderTarget->mRenderPass->RenderPass,
            VkContext->CommandPool->m_PipelineCache,
            SceneShader,
            RingBuffer
        );
        
        RenderPSO->PreparePipeline();
    }
}
