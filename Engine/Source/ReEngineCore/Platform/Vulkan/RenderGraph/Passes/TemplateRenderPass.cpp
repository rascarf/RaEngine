#include "TemplateRenderPass.h"
#include "PreDepth_frag.h"

namespace ReEngine
{
    inline void TemplateRenderPass::PreRender(VkCommandBuffer Cmd, Ref<VulkanModel> Model)
    {
       
    }

    inline void TemplateRenderPass::Render(VkCommandBuffer Cmd, Ref<VulkanModel> Model)
    {
        vkCmdBindPipeline(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS,RenderPSO->mPipeline->Pipeline);
        for(const auto& Mesh : Model->Meshes)
        {
            m_MVPData.model = Mesh->LinkNode.lock()->GetGlobalMatrix();
            RenderPSO->SetLocalUniform("uboMVP",&m_MVPData,sizeof(ModelViewProjectionBlock));
            RenderPSO->BindDescriptorSets(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS);

            Mesh->BindDraw(VkContext->GetCommandList());
        }
    }

    const std::vector<VertexAttribute>& TemplateRenderPass::GetRenderPassAttribute()
    {
        return SceneShader->perVertexAttributes;
    }

    void TemplateRenderPass::Update(const MVPBlock& MVPBLock)
    {
        m_MVPData.view = MVPBLock.view;
        m_MVPData.projection = MVPBLock.projection;
    }

    Ref<TemplateRenderPass> TemplateRenderPass::Create()
    {
        Ref<TemplateRenderPass> Result = CreateRef<TemplateRenderPass>();
        return Result;
    }

    void TemplateRenderPass::Init(Ref<VulkanDynamicBufferRing> InRingBuffer)
    {
        RingBuffer = InRingBuffer;
    }

    inline void TemplateRenderPass::Register(Ref<FrameGraphNode> Node,Ref<VulkanDevice> Device)
    {
        if(!Node->Enabled)
            return;
            
        auto Context = Renderer::GetContext().get();
        VkContext = dynamic_cast<ReEngine::VulkanContext*>(Context);
        
        SceneShader = VulkanShader::Create(Device,true,&OBJ_VERT,&PREDEPTH_FRAG,nullptr,nullptr,nullptr,nullptr);
        RenderPSO = VulkanMaterial::Create(
            Device,
            Node->RenderTarget->mRenderPass->RenderPass,
            VkContext->CommandPool->m_PipelineCache,
            SceneShader,
            RingBuffer
        );
        
        RenderPSO->PreparePipeline();
    }
}