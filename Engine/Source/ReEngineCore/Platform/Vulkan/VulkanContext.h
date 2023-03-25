#pragma once
#include "Core/PCH.h"
#include "Renderer/RHI/GraphicsContext.h"

struct  GLFWwindow;
namespace ReEngine
{
    class VulkanContext : public GraphicsContext
    {
    public:
        VulkanContext(GLFWwindow* windowHandle);

        virtual void Init() override; // Load OpenGL Context
        virtual void SwapBuffers() override;
    private:
        GLFWwindow* m_WindowHandle;
    };
}

