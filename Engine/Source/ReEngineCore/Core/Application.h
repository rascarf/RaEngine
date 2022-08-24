#pragma once
#include "PCH.h"
#include "Core.h"
#include "Event/Event.h"
#include "Window/Window.h"
#include "Layer/LayerStack.h"
#include "Layer/ImGuiLayer.h"
#include <ThirdParty/Glad/include/glad/glad.h>

#include "Core/SIngletonTemplate.h"

namespace ReEngine
{
    class Application : public SingletonTemplate<Application>
    {
    public:
        Application() = default;
        virtual ~Application(){};
        
        void OnEvent(std::shared_ptr<Event> e);

        void PushLayer(Ref<Layer> InLayer);
        void PushOverlay(Ref<Layer> Overlay);
        void PopLayer(Ref<Layer> InLayer);

        bool OnWindowClose(Ref<Event> e);
        bool OnWindowResize(Ref<Event> e);

        [[nodiscard]]Window& GetWindow() { return *m_Window; }

    public:
        void Init();
        void Run();
        void Shutdown();
        void Clean();
        
    private:
        Scope<Window> m_Window;
        LayerStack mLayerStack;
        bool mRunning = true;
        float m_LastTime;
        
        Ref<ImGuiLayer> m_UI;

    private:   
        friend void AppInit(Application& app);
    };
}
