#pragma once
#include "Core/Window/Window.h"
#include <GLFW/glfw3.h>
class GLFWwindow;

namespace ReEngine
{
    class GLWindow : public ReEngine::Window
    {
    public:
        GLWindow(const WindowProperty& Props);
        virtual ~GLWindow() override;

        void Update() override;
        bool IsVSync() const override;
        void SetVSync(bool enabled) override;
        unsigned GetWindowHeight() const override { return mData.Height; }
        unsigned GetWindowWidth() const override { return mData.Width; };
        virtual void SetEventCallback(const EventCallBackFunc CallBack) override;
        inline virtual void* GetNativeWindow() override { return m_Window; }

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
        WindowData mData;
        GLFWwindow* m_Window;
        GLFWcursor* m_MouseCursors[9] = { 0 };
    };
}



