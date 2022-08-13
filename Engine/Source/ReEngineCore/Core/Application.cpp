#include "Core/Application.h"
#include "Event/ApplicationEvent.h"
#include "Log/Log.h"
#include "Layer/Layer.h"
#include "Event/EventDispatcher.h"
#include "Input/Input.h"

#include "Platform/OpenGL/OpenGLVertexBuffer.h"
#include "Platform/OpenGL/OpenGLIndexBuffer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"


namespace ReEngine
{
    Application* Application::s_instance = nullptr;
    std::unique_ptr<Window> Application::m_Window = nullptr;

    Application::Application()
    {
        if(s_instance)
        {
            RE_CORE_ERROR("Application should be singleton");
        }

        s_instance = this;

        m_Window = std::unique_ptr<Window>(Window::CreateReWindow());

        
        m_UI = new ImGuiLayer();
        PushOverlay(m_UI);

        m_Window->SetEventCallback([this](std::shared_ptr<Event> e)
        {
                OnEvent(e);
        });
        

        float vertices[3 * 3] = {
        -0.5f,-0.5f,0.0f,
            0.5f,-0.5f,0.0f,
            0.0f,0.5f,0.0f
        };

        unsigned int indices[3] = { 0,1,2 };
        
        BufferElement VertexElement{ShaderDataType::Float3, "a_Position", false};
        BufferLayout layout{VertexElement};
        
        Ref<OpenGLVertexBuffer> vb =  CreateRef<OpenGLVertexBuffer>(vertices, sizeof(vertices));
        vb->SetLayout(layout);

        Ref<OpenGLIndexBuffer> ib = CreateRef<OpenGLIndexBuffer>(indices, sizeof(indices));
        
        VArray = CreateRef<OpenGLVertexArray>();
        VArray->AddVertexBuffer(vb);
        VArray->SetIndexBuffer(ib);
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
            
            VArray->Bind();
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);

            for (auto it = mLayerStack.end(); it != mLayerStack.begin(); )
                (*(--it))->OnUpdate();

            m_UI->Begin();
            for (Layer* it : mLayerStack)
                it->OnUIRender();
            m_UI->End();

            m_Window->Update();
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


