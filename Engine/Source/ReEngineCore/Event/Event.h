//事件抽象类
#pragma once
#include <string>
#include "Core/Core.h"
#include "Core/PCH.h"

class EventDispatcher;

//可以为每一个函数都调用这个宏来降低编码
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() {return EventType::##type;}\
				virtual EventType GetEventType() const override {return GetStaticType();}\
				virtual const char* GetName() const override {return #type;}

#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override {return category;}

namespace ReEngine
{
    enum class EventType
    {
        None = 0,
        WindowClose,
        WindowResize,
        WindowFocus,
        WindowLoseFocus,
        WindowMoved,
        AppTick,
        AppUpdate,
        AppRender,
        KeyPressed,
        KeyReleased,
        MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        MouseScrolled
    };

    //这样做是为了方便地过滤事件
    //事件的组成用或运算
    //事件的过滤用与运算
    enum class EventCategory
    {
        None = 0,
        EventCategoryApplication = BIT(0),
        EventCategoryInput = BIT(1),
        EventCategoryKeyboard = BIT(2),
        EventCategoryMouse = BIT(3),
        EventCategoryMouseButton = BIT(4)
    };

    class Event
    {
    public:
        virtual EventType GetEventType() const = 0;
        virtual const char* GetName() const = 0;
        virtual int GetCategoryFlags() const = 0;
        virtual std::string ToString() const { return GetName(); };
        inline virtual bool IsInCategory(EventCategory Category)
        {
            return GetCategoryFlags() & static_cast<int>(Category);
        }

        bool Handled = false; //事件默认没有消耗

        friend EventDispatcher;
    };
}

