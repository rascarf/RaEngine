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

        virtual void Init() override;
        virtual void SwapBuffers() override;
    private:
        GLFWwindow* m_WindowHandle;
    };
}
