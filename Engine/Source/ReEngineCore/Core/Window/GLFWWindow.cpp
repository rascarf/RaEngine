#include "GLFWWindow.h"

#include "Log/Log.h"


namespace ReEngine
{
    static bool s_GLFWInitialized = false;

    GLWindow::GLWindow(const WindowProperty& Props)
    {
        Init(Props);
    }

    GLWindow::~GLWindow()
    {
    }

    void GLWindow::Update()
    {
        glfwPollEvents();
        glfwSwapBuffers(m_Window);
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

            s_GLFWInitialized = true;
        }

        m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, mData.Title.c_str(), nullptr, nullptr);
        glfwMakeContextCurrent(m_Window);
        glfwSetWindowUserPointer(m_Window, &mData);
        SetVSync(true);

    }

    void GLWindow::ShutDonw()
    {
        glfwDestroyWindow(m_Window);
    }
}

