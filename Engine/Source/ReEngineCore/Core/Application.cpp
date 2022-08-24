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
    void Application::OnInit()
    {
        m_Window = Window::CreateReWindow(WindowProperty("ReEngine"));
        
        m_UI = std::make_shared<ImGuiLayer>(ImGuiLayer()); 
        PushOverlay(m_UI);

        m_Window->SetEventCallback([this](std::shared_ptr<Event> e)
        {
                OnEvent(e);
        });
    }
    
    void Application::Run()
    {
        OnInit();
        while (mRunning)
        {
            float CurrentTime = m_Window->GetTime();
            Timestep Ts = CurrentTime - m_LastTime;
            m_LastTime = CurrentTime;
            
            for (auto it = mLayerStack.end(); it != mLayerStack.begin(); )
                (*(--it))->OnUpdate(Ts);

            m_UI->Begin();
            for (const auto& it : mLayerStack)
                it->OnUIRender();
            m_UI->End();

            m_Window->Update(Ts);
        }

        OnShutdown();
    }
    
    void Application::OnEvent(std::shared_ptr<Event> e)
    {
        EventDispatcher Dispatcher(e);
        Dispatcher.DispatchEvent<WindowCloseEvent>([&](std::shared_ptr<Event> e) {return OnClose(e); });

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

    bool Application::OnClose(std::shared_ptr<Event> e)
    {
        mRunning = false;
        return true;
    }
}


