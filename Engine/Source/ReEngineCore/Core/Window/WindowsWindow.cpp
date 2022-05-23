#include "Core/Window/WindowsWindow.h"
#include "Core/Window/GLFWWindow.h"
#include "Event/ApplicationEvent.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"
#include "Log/Log.h"

namespace ReEngine
{
    WindowsWindow* WindowsWindow::Singleton = nullptr;

    LRESULT CALLBACK WinProc(HWND handle, UINT Msg, WPARAM wParam, LPARAM lParam);

    WindowsWindow::WindowsWindow(const ReEngine::WindowProperty& WindowProperty)
    {
        Singleton = this;
        Init(WindowProperty);
    }

    WindowsWindow::~WindowsWindow()
    {
        ShutDown();
    }

    void WindowsWindow::Update()
    {
        MSG msg = { 0 };

        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    unsigned WindowsWindow::GetWindowHeight() const
    {
        return mWindowData.Height;
    }

    unsigned WindowsWindow::GetWindowWidth() const
    {
        return mWindowData.Width;
    }

    WindowsWindow* WindowsWindow::Instance()
    {
        return Singleton;
    }

    void WindowsWindow::SetEventCallback(const EventCallBackFunc CallBack)
    {
        mWindowData.EventCallback = CallBack;
    }

    LRESULT WindowsWindow::Proc(HWND handle, UINT Msg, WPARAM wParam, LPARAM lParam)
    {
        switch(Msg)
        {
            case WM_SIZE:
            {
                auto Event = std::make_shared<WindowResizeEvent>(WindowResizeEvent(LOWORD(lParam), HIWORD(lParam)));
                if (this->mWindowData.EventCallback)
                {
                    this->mWindowData.EventCallback(Event);
                }
                break;
            }

            case WM_DESTROY:
            {
                auto DestroyEvent = std::make_shared<WindowCloseEvent>(WindowCloseEvent());
                if (this->mWindowData.EventCallback)
                {
                    this->mWindowData.EventCallback(DestroyEvent);
                }
                break;
            }
        }

        return DefWindowProc(handle, Msg, wParam, lParam);
    }

    void WindowsWindow::Init(const ReEngine::WindowProperty& WindowProperty)
    {
        mWindowData.Width = WindowProperty.Width;
        mWindowData.Height = WindowProperty.Height;
        mWindowData.Title = WindowProperty.Title;

        RE_CORE_INFO("Created Windows:{0}({1},{2})", WindowProperty.Title, WindowProperty.Width, WindowProperty.Height);

        {
            WNDCLASSEX wc;
            ZeroMemory(&wc, sizeof(WNDCLASSEX));
            Hinstance = GetModuleHandle(NULL);

            wc.cbSize = sizeof(WNDCLASSEX);
            wc.style = CS_HREDRAW | CS_VREDRAW;
            wc.hInstance = Hinstance;
            wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
            wc.hCursor = LoadCursor(NULL, IDC_ARROW);
            wc.lpszMenuName = 0;
            wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
            wc.lpszClassName = L"WindowClass";
            wc.lpfnWndProc = WinProc;
            wc.cbClsExtra = 0;
            wc.cbWndExtra = 0;

            if (!RegisterClassEx(&wc))
            {
                MessageBox(0, L"RegisterClass Failed", 0, 0);

                RE_CORE_ERROR("RegisterClass Failed");
                return;
            }
            std::wstring widestr = std::wstring(mWindowData.Title.begin(), mWindowData.Title.end());

            WinHandle = CreateWindowEx(
                0, L"WindowClass", widestr.c_str(),
                WS_OVERLAPPEDWINDOW,
                0, 0, mWindowData.Width, mWindowData.Height,
                NULL, NULL, Hinstance, NULL
            );

            ShowWindow(WinHandle, SW_SHOW);
            UpdateWindow(WinHandle);

            //裁剪区域
            RECT CanvasRect;
            ::GetClientRect(WinHandle, &CanvasRect);

            mWindowData.Height = CanvasRect.bottom - CanvasRect.top;
            mWindowData.Width = CanvasRect.right - CanvasRect.left;
        }
    }

    void WindowsWindow::ShutDown()
    {
    }

    Window* Window::CreateReWindow(const WindowProperty& Property)
    {
        return new GLWindow(Property); // GLFW
        // return new WindowsWindow(Property); // Win
    }


    LRESULT CALLBACK WinProc(HWND handle, UINT Msg, WPARAM wParam, LPARAM lParam)
    {
        return WindowsWindow::Instance()->Proc(handle, Msg, wParam, lParam);
    }
}
