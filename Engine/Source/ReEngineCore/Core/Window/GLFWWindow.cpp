#include "GLFWWindow.h"
#include "Event/Event.h"
#include "Event/ApplicationEvent.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"
#include "Log/Log.h"
#include "Platform/OpenGL/OpenGLContext.h"

#include "imgui.h"


namespace ReEngine
{
    static void GLFWErrorCallback(int error,const char* description)
    {
        RE_CORE_ERROR("GLFW Error ({0}):{1}", error, description);
    }

    static bool s_GLFWInitialized = false;

    GLWindow::GLWindow(const WindowProperty& Props)
    {
        Init(Props);
    }

    GLWindow::~GLWindow()
    {
    }

    void GLWindow::Update(Timestep ts)
    {
        glfwPollEvents();
        m_Context->SwapBuffers();
        ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
        glfwSetCursor(m_Window, m_MouseCursors[imgui_cursor] ? m_MouseCursors[imgui_cursor] : m_MouseCursors[ImGuiMouseCursor_Arrow]);
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    bool GLWindow::IsVSync() const
    {
        return mData.VSync;
    }

    void GLWindow::SetVSync(bool enabled)
    {
        if (enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);

        mData.VSync = enabled;
    }

    void GLWindow::SetEventCallback(const EventCallBackFunc CallBack)
    {
        mData.EventCallBack = CallBack;
    }

    void GLWindow::Init(const WindowProperty& props)
    {
        mData.Width = props.Width;
        mData.Height = props.Height;
        mData.Title = props.Title;

        RE_CORE_INFO("Created Windows:{0}({1},{2})", props.Title, props.Width, props.Height);

        if(!s_GLFWInitialized)
        {
            int success = glfwInit();
            glfwSetErrorCallback(GLFWErrorCallback);
            s_GLFWInitialized = true;
        }

        m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, mData.Title.c_str(), nullptr, nullptr);

        m_Context = GraphicsContext::Create(m_Window);
        m_Context->Init();
        
        glfwSetWindowUserPointer(m_Window, &mData);
        SetVSync(true);

        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
            {
                WindowData&  data  = *(WindowData*)glfwGetWindowUserPointer(window);
                auto event = std::make_shared<WindowResizeEvent>(width, height);

                data.EventCallBack(event);
                data.Width = width;
                data.Height = height;
            });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
                auto event = std::make_shared<WindowCloseEvent>();
                data.EventCallBack(event);
            });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int modes)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
                switch (action)
                {
                    case GLFW_PRESS:
                    {
                        auto keyEvent = std::make_shared<KeyPressedEvent>(key, 0);
                        data.EventCallBack(keyEvent);
                        break;
                    }

                    case GLFW_RELEASE:
                    {
                        auto keyEvent = std::make_shared<KeyReleasedEvent>(key);
                        data.EventCallBack(keyEvent);
                        break;
                    }
                    case GLFW_REPEAT:
                    {
                        auto keyEvent = std::make_shared<KeyPressedEvent>(key, 1);
                        data.EventCallBack(keyEvent);
                        break;
                    }
                }
            });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
                switch (action)
                {
                case GLFW_PRESS:
                {
                    auto MouseEvent = std::make_shared<MouseButtonPressedEvent>(button);
                    data.EventCallBack(MouseEvent);
                    break;
                }

                case GLFW_RELEASE:
                {
                    auto keyEvent = std::make_shared<MouseButtonReleasedEvent>(button);
                    data.EventCallBack(keyEvent);
                    break;
                }
                }
            });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xoffset, double yoffset)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

                auto ScrollEvent = std::make_shared<MouseScrollEvent>(xoffset, yoffset);
                data.EventCallBack(ScrollEvent);
            });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
            {
                WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        
                auto MouseEvent = std::make_shared<MouseMoveEvent>(xPos, yPos);
                data.EventCallBack(MouseEvent);
            });

        glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int codepoint)
            {
                auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

                auto TypeEvent = std::make_shared<KeyTypedEvent>((int)codepoint);
                data.EventCallBack(TypeEvent);
            });

        m_MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        m_MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
        m_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);   // FIXME: GLFW doesn't have this.
        m_MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
        m_MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        m_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
        m_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);  // FIXME: GLFW doesn't have this.
        m_MouseCursors[ImGuiMouseCursor_Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

    }

    void GLWindow::ShutDonw()
    {
        glfwDestroyWindow(m_Window);
    }
}

