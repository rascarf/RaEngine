#pragma once
#include "PCH.h"
#include "Core.h"
#include "Event/Event.h"
#include "Window/Window.h"
#include "Layer/LayerStack.h"
#include "Layer/ImGuiLayer.h"
#include <ThirdParty/Glad/include/glad/glad.h>

#include "Platform/OpenGL/OpenGLShader.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
#include "Renderer/RHI/Shader.h"
#include "Core/SIngletonTemplate.h"

namespace ReEngine
{
    class Application : public SingletonTemplate<Application>
    {
    public:
        Application() = default;

        virtual ~Application(){};

        void Run();

        virtual void OnInit();
        virtual void OnShutdown() {}
        virtual void OnUpdate() {}
        virtual void OnEvent(std::shared_ptr<Event> e);

        virtual void PushLayer(Ref<Layer> InLayer);
        virtual void PushOverlay(Ref<Layer> Overlay);

        bool OnClose(Ref<Event> e);

        [[nodiscard]]Window& GetWindow() { return *m_Window; }
    
    private:
        Scope<Window> m_Window;
        LayerStack mLayerStack;
        bool mRunning = true;
        float m_LastTime;
        
        Ref<ImGuiLayer> m_UI;
        Ref<OpenGLShader> mShader;
        Ref<OpenGLVertexArray> VArray;
    };
}
