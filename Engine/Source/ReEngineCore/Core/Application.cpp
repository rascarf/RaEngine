#include "Core/Application.h"

#include "Event/ApplicationEvent.h"
#include "Log/Log.h"
#include "Layer/Layer.h"
#include "Event/EventDispatcher.h"



namespace ReEngine
{
    Application::Application()
    {
        mWindow = std::unique_ptr<Window>(Window::CreateReWindow());

        mWindow->SetEventCallback([this](std::shared_ptr<Event>& e)
        {
                OnEvent(e);
        });
    }

    Application::~Application()
    {

    }

    void Application::Run()
    {
        while (mRunning)
        {
            glClearColor(0.0, 0.0, 1.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT);
            mWindow->Update();
        }
    }

    void Application::OnEvent(std::shared_ptr<Event> e)
    {
        RE_CORE_INFO("{0}", e->ToString());
        EventDispatcher CloseDispatcher(e);
        CloseDispatcher.DispatchEvent<WindowCloseEvent>([&](Event* e) {return OnClose(e); });

        for (auto it = mLayerStack.end(); it != mLayerStack.begin(); )
        {
            (*(--it))->OnEvent(e.get());
            if (e->Handled)
                break;
        }
        
    }

    bool Application::OnClose(Event* e)
    {
        mRunning = false;
        return true;
    }
}


