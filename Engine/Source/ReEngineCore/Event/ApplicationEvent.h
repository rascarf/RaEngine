#pragma once
#include "Event/Event.h"

namespace ReEngine
{
    class WindowResizeEvent : public Event
    {
    public:
        WindowResizeEvent(int InWidth,int InHeight):Width(InWidth),Height(InHeight){}

        inline int GetWidth() { return Width; }
        inline int GetHeight() { return Height; }

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "WindowResizeEvent: New Window Size: (" << Width << "," << Height << ")";
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryApplication))
    private:
        int Width = 1280;
        int Height = 720;
    };

    class WindowCloseEvent : public Event
    {
    public:
        WindowCloseEvent(){}

        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryApplication))
    };

    class AppTickEvent : public Event
    {
    public:
        AppTickEvent() {}

        EVENT_CLASS_TYPE(AppTick)
            EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryApplication))
    };

    class AppUpdateEvent : public Event
    {
    public:
        AppUpdateEvent() {}

        EVENT_CLASS_TYPE(AppUpdate)
            EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryApplication))
    };

    class AppRenderEvent : public Event
    {
    public:
        AppRenderEvent() {}

        EVENT_CLASS_TYPE(AppRender)
            EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryApplication))
    };

}
