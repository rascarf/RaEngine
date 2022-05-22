#pragma once
#include <sstream>
#include "Event.h"

namespace ReEngine
{
    class KeyEvent : public ReEngine::Event
    {
    public:

        KeyEvent() {}

        inline int getKeyCode() { return KeyCode; }

        EVENT_CLASS_CATEGORY(static_cast<int> (EventCategory::EventCategoryKeyboard) | static_cast<int>(EventCategory::EventCategoryInput))

    protected:
        int KeyCode = 0;

        KeyEvent(int InKeyCode):KeyCode(InKeyCode){}

    };

    class KeyPressedEvent : public KeyEvent
    {
    public:
        KeyPressedEvent(int InKeyCode, int InRepeatCount) :RepeatCount(InRepeatCount)
        {
            this->KeyCode = InKeyCode;
        }

        inline int GetRepeatCount() { return RepeatCount; }

        std::string ToString() const override
        {
            std::stringstream ss;
            ss<<"KeyPressedEvent:"<<KeyCode << "(" <<RepeatCount << " Repeats)";
            return ss.str();
        }

        EVENT_CLASS_TYPE(KeyPressed)

    private:
        int RepeatCount;
    };

    class KeyReleasedEvent : public KeyEvent
    {
    public:

        KeyReleasedEvent(int InKeyCode)
        {
            this->KeyCode = InKeyCode;
        }

        std::string ToString() const override
        {
           std::stringstream ss;
           ss << "KeyReleasedEvent: " << KeyCode;
            return ss.str();
        }

        EVENT_CLASS_TYPE(KeyReleased)
    };

}

