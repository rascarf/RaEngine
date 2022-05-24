#pragma once
#include "Event.h"
#include "Core/PCH.h"

//�¼��ַ���
//��һ���¼���һ��������
//���������returnһ��boolean���������Ƿ���������¼�
namespace ReEngine
{
    class EventDispatcher
    {
        //�����T��ʵ�������Event
        template<typename T>
        using EventFunc = std::function<bool(T)>;

    public:
        //���캯������Event��Dispatcher��
        //�����¼�����Dispathcher��ʹ��Dispatcher�ɷ��¼�
        EventDispatcher(std::shared_ptr<Event> InEvent):mEvent(InEvent){}

        //����ĺ����÷º�����װ
        template<typename T, typename F>
        bool DispatchEvent(F&& Func)
        {
            auto s = mEvent->GetEventType();
            auto a = T::GetStaticType();
            if(mEvent->GetEventType() == T::GetStaticType())
            {
                mEvent->Handled = Func(std::dynamic_pointer_cast<T>(mEvent));
                return true;

            }

            return false;
        }

    private:
        std::shared_ptr<Event> mEvent;
    };
}


