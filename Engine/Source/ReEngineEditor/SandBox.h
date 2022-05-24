#pragma once
#include "ReEngine.h"

class SandBox :public ReEngine::Application
{
public:
    SandBox();

    void OnEvent(std::shared_ptr<ReEngine::Event> e) override
    {
        // if(e->GetEventType() == ReEngine::EventType::KeyPressed)
        // {
        //     auto event = std::dynamic_pointer_cast<ReEngine::KeyPressedEvent>(e);
        //
        //     if(event->getKeyCode() == static_cast<int>(RE_KEY_F1))
        //     {
        //         RE_TRACE("true");
        //     }
        // }
    }

    ~SandBox();
};

inline ReEngine::Application*ReEngine::CreateApplication()
{
    return new SandBox();
}
