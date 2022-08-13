#pragma once
#include "PCH.h"
#include "Core.h"
#include "Event/Event.h"
#include "Window/Window.h"
#include "Layer/LayerStack.h"
#include "Layer/ImGuiLayer.h"
#include <ThirdParty/Glad/include/glad/glad.h>

#include "Platform/OpenGL/OpenGLVertexArray.h"

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

        inline static Window* GetWindow() { return m_Window.get(); }
        inline static Application& Get() { return *s_instance; }

    private:
        static Application* s_instance;
        static std::unique_ptr<Window> m_Window;
        ImGuiLayer* m_UI;
        bool mRunning = true;
        LayerStack mLayerStack;
        

        unsigned int m_VertexArray, m_VertexBuffer, m_Indices;

        Ref<OpenGLVertexArray> VArray;
    };

    //单例模式
    Application* CreateApplication();
}
