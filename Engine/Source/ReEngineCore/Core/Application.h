#pragma once
#include "PCH.h"
#include "Core.h"
#include "Event/Event.h"
#include "Window/Window.h"
#include "Layer/LayerStack.h"

#include <Glad/glad.h>

namespace ReEngine
{
    class Application
    {
    public:
        Application();

        virtual ~Application();

        void Run();

        void OnEvent(std::shared_ptr<Event> e);

        virtual void PushLayer(Layer* InLayer);

        virtual void PushOverlay(Layer* Overlay);

        bool OnClose(Event* e);

        Window* GetWindow() { return mWindow.get(); }
        inline static Application& Get() { return *s_instance; }

    private:

    private:
        std::unique_ptr<Window> mWindow;
        bool mRunning = true;
        LayerStack mLayerStack;
        static Application* s_instance;
    };

    //单例模式
    Application* CreateApplication();
}
