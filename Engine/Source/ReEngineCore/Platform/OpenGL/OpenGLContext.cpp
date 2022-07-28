#include "Platform/OpenGL/OpenGLContext.h"

#include "GLFW/glfw3.h"
#include "Glad/glad.h"

#include "Log/Log.h"

namespace ReEngine
{
    ReEngine::OpenGLContext::OpenGLContext(GLFWwindow* windowHandle):m_WindowHandle(windowHandle)
    {

    }

    void OpenGLContext::Init()
    {
        glfwMakeContextCurrent(m_WindowHandle);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            RE_CORE_ERROR("Failed to initailize Glad");
            return;
        }
    }

    void OpenGLContext::SwapBuffers()
    {
        glfwSwapBuffers(m_WindowHandle);
    }
}

