#pragma once
#include "Core/Core.h"
#include "Core/PCH.h"
#include "Event/Event.h"

namespace ReEngine
{
    using EventCallBackFunc = std::function<void(std::shared_ptr<Event>)>;

    struct WindowProperty
    {
        std::string  Title;
        unsigned int Width;
        unsigned int Height;

        WindowProperty(std::string InTitle = "ReEngine",unsigned int InWidth = 1280,unsigned int InHeight = 720):Title(InTitle),Width(InWidth),Height(InHeight){}
    };

    class Window
    {
    public:

        virtual ~Window() {};

        virtual void Update() = 0;

        virtual unsigned int GetWindowWidth() const = 0;
        virtual unsigned int GetWindowHeight() const = 0;

        virtual void SetEventCallback(const EventCallBackFunc CallBack) = 0;
        static Window* CreateReWindow(const WindowProperty& Property = WindowProperty());
        virtual void SetVSync(bool enabled) = 0;
        virtual bool IsVSync() const = 0;
    };
};


