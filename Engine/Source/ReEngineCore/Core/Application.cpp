#include "Core/Application.h"
#include "Event/ApplicationEvent.h"
#include "Log/Log.h"
#include "Layer/Layer.h"
#include "Event/EventDispatcher.h"
#include "GLFW/glfw3.h"
#include "Input/Input.h"

#include "Platform/OpenGL/OpenGLVertexBuffer.h"
#include "Platform/OpenGL/OpenGLIndexBuffer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Renderer/RHI/RenderCommand.h"
#include "Renderer/RHI/Renderer.h"
#include "Window/WindowsWindow.h"

namespace ReEngine
{
    void Application::Init()
    {
        m_Window = Window::CreateReWindow(m_WindowProperty);
        
        m_Window->SetEventCallback([this](std::shared_ptr<Event> e)
        {
                OnEvent(e);
        });
    }
    
    void Application::Run()
    {
        while (mRunning)
        {
            float CurrentTime = m_Window->GetTime();
            Timestep Ts = CurrentTime - m_LastTime;
            m_LastTime = CurrentTime;

            m_Window->PollEvent();
            
            //更新数据
            auto Context = Renderer::GetContext().get();
            auto VulkanContext = dynamic_cast<ReEngine::VulkanContext*>(Context);
            
            VulkanContext->Acquire();
            
            for (auto it = mLayerStack.end(); it != mLayerStack.begin(); )
                (*(--it))->OnUpdate(Ts);
            
            VulkanContext->BeginUI();
            for (auto it = mLayerStack.end(); it != mLayerStack.begin(); )
                (*(--it))->OnUIRender(Ts);
            VulkanContext->EndUI();
            
            for (auto it = mLayerStack.end(); it != mLayerStack.begin(); )
                (*(--it))->OnRender();
            
            //present
            VulkanContext->SwapBuffers(Ts);
            
            m_Window->Update(Ts);
        }

        for (auto it = mLayerStack.end(); it != mLayerStack.begin(); )
        {
            (*(--it))->OnDetach();
        }
    }

    void Application::Shutdown()
    {
        m_Window->ShutDown();
    }

    void Application::Clean()
    {
        
    }

    void Application::OnEvent(std::shared_ptr<Event> e)
    {
        EventDispatcher Dispatcher(e);
        Dispatcher.DispatchEvent<WindowCloseEvent>([&](std::shared_ptr<Event> e) {return OnWindowClose(e); });
        Dispatcher.DispatchEvent<WindowResizeEvent>([&](std::shared_ptr<WindowResizeEvent> e) {return OnWindowResize(e); });

        for (auto it = mLayerStack.end(); it != mLayerStack.begin(); )
        {
            (*(--it))->OnEvent(e);
            if (e->Handled)
                break;
        }
    }

    void Application::PushLayer(Ref<Layer> InLayer)
    {
        mLayerStack.PushLayer(InLayer);
        InLayer->OnAttach();
    }

    void Application::PushOverlay(Ref<Layer> Overlay)
    {
        mLayerStack.PushOverlay(Overlay);
        Overlay->OnAttach();
    }

    void Application::PopLayer(Ref<Layer> InLayer)
    {
        mLayerStack.PopLayer(InLayer);
        InLayer->OnDetach();
    }

    bool Application::OnWindowResize(Ref<WindowResizeEvent> e)
    {
        RE_INFO("({0},{1})",e->GetWidth(),e->GetHeight());

        // Renderer::OnWindowResize(e->GetWidth(),e->GetHeight());
        m_WindowProperty.Height = e->GetHeight();
        m_WindowProperty.Width = e->GetWidth();
        
        auto Context = Renderer::GetContext().get();
        auto VulkanContext = dynamic_cast<ReEngine::VulkanContext*>(Context);
        VulkanContext-> RecreateSwapChain();
        
        return false;
    }

    bool Application::OnWindowClose(std::shared_ptr<Event> e)
    {
        mRunning = false;
        return true;
    }
}


