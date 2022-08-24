#include "Core/Application.h"
#include "Event/ApplicationEvent.h"
#include "Log/Log.h"
#include "Layer/Layer.h"
#include "Event/EventDispatcher.h"
#include "GLFW/glfw3.h"
#include "Input/Input.h"

#include "Platform/OpenGL/OpenGLVertexBuffer.h"
#include "Platform/OpenGL/OpenGLIndexBuffer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
#include "Renderer/RHI/RenderCommand.h"
#include "Renderer/RHI/Renderer.h"
#include "Window/WindowsWindow.h"


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

        std::string VertexSrc = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec4 a_Color;

        out vec3 v_Pos;
        out vec4 v_Color;

        void main()
        {
            v_Pos = aPos;
            v_Color = a_Color;
            gl_Position = vec4(aPos,1.0);
        })";

        std::string FragmentSrc = R"(
        #version 330 core
        layout(location = 0) out vec4 color;
        in vec3 v_Pos;
        in vec4 v_Color;

        uniform vec4 colorMatrix;
        void main()
        {
            color = vec4(v_Pos * 0.5 + 0.5,1.0);
            color = colorMatrix;
        })";

        mShader = CreateRef<OpenGLShader>("TestShader",VertexSrc, FragmentSrc);
        
        

        float vertices[3 * 7] = {
        -0.5f,-0.5f,0.0f,1.0f,1.0f,0.0f,1.0f,
            0.5f,-0.5f,0.0f,0.5f,0.5f,0.5f,1.0f,
            0.0f,0.5f,0.0f,1.0f,1.0f,1.0f,0.1f
        };

        unsigned int indices[3] = { 0,1,2 };
        
        BufferElement VertexElement{ShaderDataType::Float3, "aPos", false};
        BufferElement ColorElement{ShaderDataType::Float4, "a_Color", false};

        BufferLayout layout{VertexElement,ColorElement};
        
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
            float CurrentTime = m_Window->GetTime();
            Timestep Ts = CurrentTime - m_LastTime;
            m_LastTime = CurrentTime;
            
            RenderCommand::SetClearColor(glm::vec4(0.0, 0.0, 1.0, 1.0));
            RenderCommand::Clear();
            
            //RenderCommand::DrawIndexed(VArray);
            // Renderer::Submit(VArray);
            RE_INFO("{}", 1.0f/Ts.GetSeconds());

            for (auto it = mLayerStack.end(); it != mLayerStack.begin(); )
                (*(--it))->OnUpdate(Ts);

            m_UI->Begin();
            for (Layer* it : mLayerStack)
                it->OnUIRender();
            m_UI->End();

            m_Window->Update(Ts);
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


