#pragma once
#include "Core/Core.h"
#include "Window.h"

#include <Windows.h>

namespace ReEngine
{
    struct WindowData
    {
        std::string Title;
        unsigned int Width, Height;
        EventCallBackFunc EventCallback;
    };

    class WindowsWindow :public ReEngine::Window
    {
    public:
        WindowsWindow(const ReEngine::WindowProperty& WindowProperty);
        virtual ~WindowsWindow();

        virtual void Update() override;

        inline virtual unsigned int GetWindowHeight() const override;
        inline virtual unsigned int GetWindowWidth() const override;

        static WindowsWindow* Instance();

        virtual void SetEventCallback(const EventCallBackFunc CallBack) override;

        LRESULT  Proc(HWND handle, UINT Msg, WPARAM wParam, LPARAM lParam);

        bool IsVSync() const override{return true;}
        void SetVSync(bool enabled) override{}

    private:
        void Init(const ReEngine::WindowProperty&);
        void ShutDown();

    private:
        HINSTANCE Hinstance;
        HWND WinHandle;

        static WindowsWindow* Singleton;

        WindowData mWindowData;
    };
}

