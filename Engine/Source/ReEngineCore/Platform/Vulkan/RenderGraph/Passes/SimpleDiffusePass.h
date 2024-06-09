#pragma once

#include "Core/Core.h"
#include "FrameWork/Scene.h"
#include "Platform/Vulkan/VulkanGraphicPipeline.h"
#include "Platform/Vulkan/RenderGraph/RenderGraph.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanDynamicBufferRing.h"

namespace ReEngine
{
    class SimpleDiffusePass : public FrameGraphRenderPass
    {
    public:
        static Ref<SimpleDiffusePass> Create();
        virtual void Init(Ref<VulkanDynamicBufferRing> RingBuffer);
        virtual void OnRegister(Ref<FrameGraphNode> Node, Ref<VulkanDevice> Device) override;
        virtual std::vector<VertexAttribute> GetRenderPassAttribute();
        
        virtual void PreRender(VkCommandBuffer CmdBuffer, VulkanScene* Scene)override;
        virtual void Render(VkCommandBuffer CmdBuffer, VulkanScene* Scene)override;
        
        virtual ~SimpleDiffusePass() override
        {
            SceneShader.reset();
            RenderPSO.reset();
            RingBuffer.reset();
        }

        struct ModelViewProjectionBlock
        {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 projection;
        }m_MVPData;
        
        Ref<VulkanShader> SceneShader = nullptr;
        Ref<VulkanGraphicPipeline> RenderPSO = nullptr;
        Ref<VulkanDynamicBufferRing> RingBuffer = nullptr;
        
        ReEngine::VulkanContext* VkContext = nullptr;

        std::vector<entt::entity> RenderEntity;
    };
}

