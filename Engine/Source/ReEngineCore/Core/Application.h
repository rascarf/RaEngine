#pragma once
#include "PCH.h"
#include "Core.h"
#include "Event/Event.h"
#include "Window/Window.h"
#include "Layer/LayerStack.h"

#include <GLFW/glfw3.h>

namespace ReEngine
{
    class Application
    {
    public:
        Application();

        virtual ~Application();

        void Run();

        void OnEvent(std::shared_ptr<Event> e);

        void PushLayer(Layer* InLayer);

        void PushOverlay(Layer* Overlay);

        bool OnClose(Event* e);

    private:

    private:
        std::unique_ptr<Window> mWindow;
        bool mRunning = true;
        LayerStack mLayerStack;
    };

    //单例模式
    Application* CreateApplication();
}
