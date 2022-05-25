#include "Core/Application.h"
#include "Event/ApplicationEvent.h"
#include "Log/Log.h"
#include "Layer/Layer.h"
#include "Event/EventDispatcher.h"
#include "Input/Input.h"



namespace ReEngine
{
    Application* Application::s_instance = nullptr;
    std::unique_ptr<Window> Application:: mWindow = nullptr;

    Application::Application()
    {
        if(s_instance)
        {
            RE_CORE_ERROR("Application should be singleton");
        }

        s_instance = this;

        mWindow = std::unique_ptr<Window>(Window::CreateReWindow());

        
        m_UI = new ImGuiLayer();
        PushOverlay(m_UI);

        mWindow->SetEventCallback([this](std::shared_ptr<Event> e)
        {
                OnEvent(e);
        });

        
    }

    Application::~Application()
    {

    }

    void Application::Run()
    {
        OnInit();
        while (mRunning)
        {
            glClearColor(0.0, 0.0, 1.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT);

            for (auto it = mLayerStack.end(); it != mLayerStack.begin(); )
                (*(--it))->OnUpdate();

            m_UI->Begin();
            for (Layer* it : mLayerStack)
                it->OnUIRender();
            m_UI->End();

            mWindow->Update();
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

    void Application::PushLayer(Layer* InLayer)
    {
        mLayerStack.PushLayer(InLayer);
        InLayer->OnAttach();
    }

    void Application::PushOverlay(Layer* Overlay)
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


