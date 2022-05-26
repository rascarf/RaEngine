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

        virtual void OnInit() {}
        virtual void OnShutdown() {}
        virtual void OnUpdate() {}
        virtual void OnEvent(std::shared_ptr<Event> e);

        virtual void PushLayer(Layer* InLayer);

        virtual void PushOverlay(Layer* Overlay);

        bool OnClose(std::shared_ptr<Event> e);

        inline static Window* GetWindow() { return mWindow.get(); }

        inline static Application& Get() { return *s_instance; }

    private:
        static std::unique_ptr<Window> mWindow;
        bool mRunning = true;
        LayerStack mLayerStack;
        static Application* s_instance;

        unsigned int m_VertexArray, m_VertexBuffer, m_Indices;
    };

    //单例模式
    Application* CreateApplication();
}
