#include "SimpleDiffusePass.h"

#include <obj_frag.h>
#include <obj_vert.h>

#include "FrameWork/Component/TransformComponent.h"
#include "Renderer/RHI/Renderer.h"
#include "Platform/Vulkan/VulkanScene.h"
#include "Platform/Vulkan/Component/StaticMeshComponent.h"

Ref<SimpleDiffusePass> SimpleDiffusePass::Create()
{
    return CreateRef<SimpleDiffusePass>();
}

void SimpleDiffusePass::Init(Ref<VulkanDynamicBufferRing> InRingBuffer)
{
    RingBuffer = InRingBuffer;
}

void SimpleDiffusePass::OnRegister(Ref<FrameGraphNode> Node, Ref<VulkanDevice> Device)
{
    if(!Node->Enabled)
        return;
            
    auto Context = Renderer::GetContext().get();
    VkContext = dynamic_cast<ReEngine::VulkanContext*>(Context);
        
    SceneShader = VulkanShader::Create(Device,true,&OBJ_VERT,&OBJ_FRAG,nullptr,nullptr,nullptr,nullptr,true);
    RenderPSO = VulkanGraphicPipeline::Create(
        Device,
        Node->RenderTarget->mRenderPass->RenderPass,
        VkContext->CommandPool->m_PipelineCache,
        SceneShader,
        RingBuffer
    );

    RenderPSO->mPipelineInfo.DepthStencilState.depthTestEnable = VK_TRUE;
    RenderPSO->mPipelineInfo.DepthStencilState.depthWriteEnable = VK_FALSE;
    RenderPSO->mPipelineInfo.DepthStencilState.depthCompareOp = VkCompareOp::VK_COMPARE_OP_EQUAL;
    RenderPSO->PreparePipeline();
}

std::vector<VertexAttribute> SimpleDiffusePass::GetRenderPassAttribute()
{
    if(RenderPSO)
    {
        return RenderPSO->mShader->perVertexAttributes;
    }

    return std::vector<VertexAttribute>();
}

void SimpleDiffusePass::PreRender(VkCommandBuffer CmdBuffer, VulkanScene* Scene)
{
    RenderEntity.clear();
    
    const auto View = Scene->mScene.mRegistry.view<StaticMeshComponent,TransformComponent>();
    for(auto& Entity:View)
    {
        RenderEntity.emplace_back(Entity);
    }

    std::sort(RenderEntity.begin(),RenderEntity.end(),[&View](const entt::entity& A,const entt::entity& B)
    {
        return View.get<StaticMeshComponent>(A).PbrMaterialParamIndex < View.get<StaticMeshComponent>(B).PbrMaterialParamIndex;
    });
    
    for(auto& Entity: View)
    {
        if(RenderPSO->DescriptorSets.contains(std::to_string(View.get<StaticMeshComponent>(Entity).PbrMaterialParamIndex)))
        {
            continue;
        }
        
        RenderPSO->Prepare(std::to_string(View.get<StaticMeshComponent>(Entity).PbrMaterialParamIndex));
    }

    m_MVPData.projection = Scene->SceneCamera->GetProjection();
    m_MVPData.view = Scene->SceneCamera->GetViewMatrix();
}

void SimpleDiffusePass::Render(VkCommandBuffer CmdBuffer, VulkanScene* Scene)
{
    vkCmdBindPipeline(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS,RenderPSO->mPipeline->Pipeline);
        
    auto View =  Scene->mScene.mRegistry.view<StaticMeshComponent,TransformComponent>();

    for(const auto& Entity : RenderEntity)
    {
        if(!View.get<StaticMeshComponent>(Entity).Mesh.expired())
        {
            const auto MaterialIndex = View.get<StaticMeshComponent>(Entity).PbrMaterialParamIndex;
            m_MVPData.model = View.get<TransformComponent>(Entity).GetTransform();

            if(Scene->PBRTextureParam.size() > MaterialIndex)
            {
                if(!Scene->PBRTextureParam[MaterialIndex].DiffuseTexture.expired())
                {
                    RenderPSO->SetTexture("DiffuseMap",Scene->PBRTextureParam[MaterialIndex].DiffuseTexture.lock(),std::to_string(MaterialIndex));
                }
            }
            
            RenderPSO->SetUniform("uboMVP",&m_MVPData,sizeof(ModelViewProjectionBlock));
            RenderPSO->BindDescriptorSets(VkContext->GetCommandList(),VK_PIPELINE_BIND_POINT_GRAPHICS,std::to_string(MaterialIndex));
            
            View.get<StaticMeshComponent>(Entity).Mesh.lock()->BindDraw(CmdBuffer,SceneShader->OutputAttributes);
        }
    }
}
