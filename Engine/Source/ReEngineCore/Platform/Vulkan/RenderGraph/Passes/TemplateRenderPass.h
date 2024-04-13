#include <obj_frag.h>
#include <obj_vert.h>

#include "Core/Core.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/RenderGraph/RenderGraph.h"
#include "Platform/Vulkan/VulkanShader/VulkanShader.h"
#include "Renderer/RHI/Renderer.h"


namespace ReEngine
{
    class TemplateRenderPass : public FrameGraphRenderPass
    {
    public:
        virtual void Register(Ref<FrameGraphNode> Node,Ref<VulkanDevice> Device) override;
        virtual void PreRender(Ref<VulkanCommandBuffer> Cmd, Ref<VulkanModel> Model) override;
        virtual void Render(Ref<VulkanCommandBuffer> Cmd, Ref<VulkanModel> Model) override;

        virtual ~TemplateRenderPass() override
        {
            SceneShader.reset();
            SceneMaterial.reset();
        }

        struct ModelViewProjectionBlock
        {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 projection;
        }m_MVPData;
        
        Ref<VulkanShader> SceneShader;
        Ref<VulkanMaterial> SceneMaterial;
        Ref<VulkanDynamicBufferRing> RingBuffer;
        
        ReEngine::VulkanContext* VkContext = nullptr;
    };
}
