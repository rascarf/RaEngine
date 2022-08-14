#pragma once
#include "Core/PCH.h"
#include "Renderer/GraphicsContext.h"

struct  GLFWwindow;
namespace ReEngine
{
    class OpenGLContext : public GraphicsContext
    {
    public:
        OpenGLContext(GLFWwindow* windowHandle);

        virtual void Init() override; // Load OpenGL Context
        virtual void SwapBuffers() override;
    private:
        GLFWwindow* m_WindowHandle;
    };
}
