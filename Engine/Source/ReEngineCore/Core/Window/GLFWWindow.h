#pragma once
#include "Core/Window/Window.h"
#include "Renderer/RHI/GraphicsContext.h"

#define  GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class GLFWwindow;

namespace ReEngine
{
    class GLWindow : public ReEngine::Window
    {
    public:
        GLWindow(const WindowProperty& Props);
        virtual ~GLWindow() override;

        void Update(Timestep ts) override;
        bool IsVSync() const override;
        void SetVSync(bool enabled) override;
        unsigned GetWindowHeight() const override { return mData.Height; }
        unsigned GetWindowWidth() const override { return mData.Width; };
        virtual void SetEventCallback(const EventCallBackFunc CallBack) override;
        inline virtual void* GetNativeWindow() override { return m_Window; }
        [[nodiscard]]virtual Ref<GraphicsContext>GetGraphicsContext() const override{return m_Context;}
        [[nodiscard]]inline virtual float GetTime() override{return glfwGetTime();}
    private:

        virtual  void Init(const WindowProperty& props);
        virtual void ShutDonw();

        struct WindowData
        {
            std::string Title;
            unsigned int Width, Height;
            bool VSync;

            EventCallBackFunc EventCallBack;
        };

    private:
        Ref<GraphicsContext> m_Context;
        WindowData mData;
        GLFWwindow* m_Window;
        GLFWcursor* m_MouseCursors[9] = { 0 };
    };
}



