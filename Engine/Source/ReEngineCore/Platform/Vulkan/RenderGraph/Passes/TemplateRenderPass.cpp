#include "TemplateRenderPass.h"

namespace ReEngine
{
    inline void TemplateRenderPass::PreRender(Ref<VulkanCommandBuffer> Cmd, Ref<VulkanModel> Model)
    {
       
    }

    inline void TemplateRenderPass::Render(Ref<VulkanCommandBuffer> Cmd, Ref<VulkanModel> Model)
    {
        vkCmdBindPipeline(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS,SceneMaterial->mPipeline->Pipeline);
        for(const auto& Mesh : Model->Meshes)
        {
            m_MVPData.model = Mesh->LinkNode.lock()->GetGlobalMatrix();
            SceneMaterial->SetLocalUniform("uboMVP",&m_MVPData,sizeof(ModelViewProjectionBlock));
            SceneMaterial->BindDescriptorSets(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS);

            Mesh->BindDraw(VkContext->GetCommandList());
        }
    }

    inline void TemplateRenderPass::Register(Ref<FrameGraphNode> Node,Ref<VulkanDevice> Device)
    {
        auto Context = Renderer::GetContext().get();
        VkContext = dynamic_cast<ReEngine::VulkanContext*>(Context);
        
        SceneShader = VulkanShader::Create(Device,true,&OBJ_VERT,&OBJ_FRAG,nullptr,nullptr,nullptr,nullptr);
        SceneMaterial = VulkanMaterial::Create(
            Device,
            Node->RenderTarget->mRenderPass->RenderPass,
            VkContext->CommandPool->m_PipelineCache,
            SceneShader,
            RingBuffer
        );
        
        SceneMaterial->PreparePipeline();
    }
}