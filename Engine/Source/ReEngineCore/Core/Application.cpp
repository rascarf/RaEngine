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
#include "Renderer/RHI/RenderCommand.h"
#include "Renderer/RHI/Renderer.h"
#include "Window/WindowsWindow.h"

namespace ReEngine
{
    void Application::Init()
    {
        m_Window = Window::CreateReWindow(WindowProperty("ReEngine"));
        
        // m_UI = std::make_shared<ImGuiLayer>(ImGuiLayer()); 
        // PushOverlay(m_UI);

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
            
            // for (auto it = mLayerStack.end(); it != mLayerStack.begin(); )
            //     (*(--it))->OnUpdate(Ts);

            for(auto it : mLayerStack)
            {
                it->OnUpdate(Ts);
            }

            // m_UI->BeginUIRender();
            // for(auto it : mLayerStack)
            // {
            //     it->OnUIRender(Ts);
            // }
            // m_UI->EndUIRender();

            m_Window->Update(Ts);
        }
    }

    void Application::Shutdown()
    {
        mRunning = false;
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
        
        return false;
    }

    bool Application::OnWindowClose(std::shared_ptr<Event> e)
    {
        mRunning = false;
        return true;
    }
}


