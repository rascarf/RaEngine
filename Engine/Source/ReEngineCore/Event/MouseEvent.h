#pragma once
#include "Event.h"

namespace ReEngine
{
    class MouseMoveEvent : public Event
    {
    public:
        inline float GetX() { return MouseX; }
        inline float GetY() { return MouseY; }

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "MouseMovedEvent: (" << MouseX << "," << MouseY << ")";
            return ss.str();
        }

        EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryInput)|static_cast<int>(EventCategory::EventCategoryMouse))

        EVENT_CLASS_TYPE(MouseMoved)


    private:
        float MouseX = 0.0f;
        float MouseY = 0.0f;
    };

    class MouseScrollEvent : public Event
    {
    public:
            MouseScrollEvent(float InMouseXOffset,float InMouseYOffset):MouseXOffset(InMouseXOffset),MouseYOffset(InMouseYOffset){}

            MouseScrollEvent(){}

            inline float GetXOffset() const { return MouseXOffset; }
            inline float GetYOffset() const { return MouseYOffset; }

            std::string ToString() const override
            {
                std::stringstream ss;
                ss << "MouseScrollEvent: (" << MouseXOffset << "," << MouseYOffset << ")";
                return ss.str();
            }

            EVENT_CLASS_TYPE(MouseScrolled)
            EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryInput) | static_cast<int>(EventCategory::EventCategoryMouse))


    private:
        float MouseXOffset = 0.0f;
        float MouseYOffset = 0.0f;
    };

    class MouseButtonEvent : Event
    {
    public:
        inline int GetMouseButton()const { return MouseButton; }

        EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryInput) | static_cast<int>(EventCategory::EventCategoryMouse)|static_cast<int>(EventCategory::EventCategoryMouseButton))

    protected:
        MouseButtonEvent(int InMouseButton) :MouseButton(InMouseButton) {}

        MouseButtonEvent() {}

        int MouseButton = 0;
    };

    class MouseButtonPressedEvent:public MouseButtonEvent
    {
    public:
        MouseButtonPressedEvent(int InMouseButton)
        {
            this->MouseButton = InMouseButton;
        }

        std::string ToString() const override
        {
            std::stringstream ss;
            ss << "MouseButtonPressedEvent: " << MouseButton;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseButtonPressed)
    };

    class MouseButtonReleasedEvent :public MouseButtonEvent
    {
    public:
        MouseButtonReleasedEvent(int InMouseButton)
        {
            this->MouseButton = InMouseButton;
        }
        
        std::string ToString() const override {
            std::stringstream ss;
            ss << "MouseButtonReleasedEvent: " << MouseButton;
            return ss.str();
        }

        EVENT_CLASS_TYPE(MouseButtonReleased)
    };
}
