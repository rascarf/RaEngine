#include <obj_frag.h>
#include <obj_vert.h>

#include "Core/Core.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanGraphicPipeline.h"
#include "Platform/Vulkan/RenderGraph/RenderGraph.h"
#include "Platform/Vulkan/VulkanShader/VulkanShader.h"
#include "Platform/Vulkan/VulkanScene.h"
#include "ReEngineEditor/Layers/GraphicalLayer.h"


namespace ReEngine
{
    class PreDepthRenderPass : public FrameGraphRenderPass
    {
    public:

        static Ref<PreDepthRenderPass> Create();
        
        virtual void Init(Ref<VulkanDynamicBufferRing> RingBuffer);
        virtual void Register(Ref<FrameGraphNode> Node,Ref<VulkanDevice> Device) override;
        virtual const std::vector<VertexAttribute>& GetRenderPassAttribute();

        virtual void Update(const MVPBlock& MVPBLock);
        
        virtual void PreRender(VkCommandBuffer Cmd, Ref<VulkanModel> Model) override;
        virtual void Render(VkCommandBuffer Cmd, Ref<VulkanModel> Model) override;

        virtual void PreRender(VkCommandBuffer CmdBuffer, VulkanScene* Scene)override;
        virtual void Render(VkCommandBuffer CmdBuffer, VulkanScene* Scene)override;
        virtual void PostRender(VkCommandBuffer CmdBuffer, VulkanScene* Scene)override;
        
        virtual ~PreDepthRenderPass() override
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
    };
}
