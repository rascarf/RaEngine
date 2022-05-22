#pragma once
#include "Event.h"
#include "Core/PCH.h"

//事件分发器
//将一个事件和一个函数绑定
//这个函数会return一个boolean，来决定是否消费这个事件
namespace ReEngine
{
    class EventDispatcher
    {
        //这里的T是实例化后的Event
        template<typename T>
        using EventFunc = std::function<bool(T&)>;

    public:
        //构造函数：将Event和Dispatcher绑定
        //目前每个 Dispatcher 只绑定一个函数
        EventDispatcher(std::shared_ptr<Event>& InEvent):mEvent(InEvent){}

        //输入的函数用仿函数封装
        template<typename T, typename F>
        bool DispatchEvent(F&& Func)
        {
            if(mEvent->GetEventType() == T::GetStaticType())
            {
                mEvent->Handled = Func(static_cast<Event*>(mEvent.get()));
                return true;
            }

            return false;
        }

    private:
        std::shared_ptr<Event> mEvent;
    };
}


