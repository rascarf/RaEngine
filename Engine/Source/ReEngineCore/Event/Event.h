//�¼�������
#pragma once
#include <string>
#include "Core/Core.h"
#include "Core/PCH.h"

class EventDispatcher;

//����Ϊÿһ����������������������ͱ���
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

    //��������Ϊ�˷���ع����¼�
    //�¼�������û�����
    //�¼��Ĺ�����������
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

        bool Handled = false; //�¼�Ĭ��û������

        friend EventDispatcher;
    };
}

