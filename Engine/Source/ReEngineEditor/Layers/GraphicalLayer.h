#pragma once

#include "Layer/Layer.h"
#include "Platform/Vulkan/VulkanPipelineInfo.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanDynamicBufferRing.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanFrameBuffer.h"

class GraphicalLayer : public ReEngine::Layer
{
public:
    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual void OnEvent(std::shared_ptr<ReEngine::Event> e) override;
    virtual void OnUpdate(ReEngine::Timestep ts) override;

    virtual void OnRender() override;
    virtual void OnUIRender(ReEngine::Timestep ts) override;

    virtual void OnCreateBackBuffer();
    virtual void OnInit(){}
    virtual void OnDeInit(){}
    virtual void OnChangeWindowSize(std::shared_ptr<ReEngine::Event> e);

protected:
    ReEngine::VulkanContext* VkContext;
    Ref<VulkanBackBuffer> FrameBuffer;
};
