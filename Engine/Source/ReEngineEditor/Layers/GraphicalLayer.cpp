#include "GraphicalLayer.h"
#include "Renderer/RHI/Renderer.h"
#include "Platform/Vulkan/VulkanContext.h"

void GraphicalLayer::OnCreateBackBuffer()
{
    FrameBuffer = CreateRef<VulkanBackBuffer>(VkContext->WinProperty->Width,VkContext->WinProperty->Height,VkContext->WinProperty->Title.c_str());
    FrameBuffer->Init(VkContext);
}

void GraphicalLayer::OnAttach()
{
    auto Context = Renderer::GetContext().get();
    VkContext = dynamic_cast<ReEngine::VulkanContext*>(Context);

    OnCreateBackBuffer();
    OnInit();
}

void GraphicalLayer::OnDetach()
{
    OnDeInit();
    
    FrameBuffer->ShutDown();
}

void GraphicalLayer::OnEvent(std::shared_ptr<ReEngine::Event> e)
{
    if(e->GetEventType() == ReEngine::EventType::WindowResize)
    {
        FrameBuffer->ShutDown();
        FrameBuffer->Init(VkContext);
        OnChangeWindowSize(e);
    }
}

void GraphicalLayer::OnUpdate(ReEngine::Timestep ts)
{
    Layer::OnUpdate(ts);
}

void GraphicalLayer::OnRender()
{
    Layer::OnRender();
}

void GraphicalLayer::OnUIRender(ReEngine::Timestep ts)
{
    Layer::OnUIRender(ts);
}

void GraphicalLayer::OnChangeWindowSize(std::shared_ptr<ReEngine::Event> e)
{
}
