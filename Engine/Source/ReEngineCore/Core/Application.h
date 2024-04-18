#pragma once
#include "PCH.h"
#include "Core.h"
#include "TaskScheduler.h"
#include "Event/Event.h"
#include "Window/Window.h"
#include "Layer/LayerStack.h"
#include "Layer/ImGuiLayer.h"
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
        bool OnWindowResize(Ref<WindowResizeEvent> e);
        enki::TaskScheduler* GetTaskSheduler(); // 保证在生命周期之内是非空的

        [[nodiscard]]Window& GetWindow() { return *m_Window; }
        [[nodiscard]]const WindowProperty GetWindowInfo(){return m_WindowProperty;}

    public:
        void Init();
        void Run();
        void Shutdown();
        void Clean();
        
    private:
        WindowProperty m_WindowProperty;
        Scope<Window> m_Window;
        LayerStack mLayerStack;

        enki::TaskScheduler* TaskScheduler;
        bool mRunning = true;
        float m_LastTime;

    private:   
        friend void AppInit(Application& app);
    };
}
