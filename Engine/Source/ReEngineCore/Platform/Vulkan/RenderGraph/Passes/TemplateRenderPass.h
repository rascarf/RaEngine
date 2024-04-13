#include <obj_frag.h>
#include <obj_vert.h>

#include "Core/Core.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/RenderGraph/RenderGraph.h"
#include "Platform/Vulkan/VulkanShader/VulkanShader.h"
#include "ReEngineEditor/Layers/GraphicalLayer.h"
#include "Renderer/RHI/Renderer.h"


namespace ReEngine
{
    class TemplateRenderPass : public FrameGraphRenderPass
    {
    public:

        static Ref<TemplateRenderPass> Create();
        
        virtual void Init(Ref<VulkanDynamicBufferRing> RingBuffer);
        virtual void Register(Ref<FrameGraphNode> Node,Ref<VulkanDevice> Device) override;
        virtual const std::vector<VertexAttribute>& GetRenderPassAttribute();

        virtual void Update(const MVPBlock& MVPBLock);
        
        virtual void PreRender(VkCommandBuffer Cmd, Ref<VulkanModel> Model) override;
        virtual void Render(VkCommandBuffer Cmd, Ref<VulkanModel> Model) override;

        virtual ~TemplateRenderPass() override
        {
            SceneShader.reset();
            RenderPSO.reset();
        }

        struct ModelViewProjectionBlock
        {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 projection;
        }m_MVPData;
        
        Ref<VulkanShader> SceneShader = nullptr;
        Ref<VulkanMaterial> RenderPSO = nullptr;
        Ref<VulkanDynamicBufferRing> RingBuffer = nullptr;
        
        ReEngine::VulkanContext* VkContext = nullptr;
    };
}
