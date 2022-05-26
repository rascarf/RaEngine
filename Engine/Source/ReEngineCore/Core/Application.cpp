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

        glGenVertexArrays(1, &m_VertexArray);
        glBindVertexArray(m_VertexArray);

        glGenBuffers(1, &m_VertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);

        float vertices[3 * 3] = {
        -0.5f,-0.5f,0.0f,
            0.5f,-0.5f,0.0f,
            0.0f,0.5f,0.0f
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,3 * sizeof(float),nullptr);

        glGenBuffers(1, &m_Indices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Indices);

        unsigned int indices[3] = { 0,1,2 };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
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

            // glBindVertexArray(m_VertexArray);
            // glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

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


