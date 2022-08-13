#pragma once
#include "Event/Event.h"
#include "Core/Core.h"

namespace ReEngine
{
    class Layer
    {
    public:
        Layer(const std::string& name = "Layer");
        virtual ~Layer();

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate() {}
        virtual void OnEvent(std::shared_ptr<Event> e) {}
        virtual void Begin(){}
        virtual void End(){}
        virtual void OnUIRender(){}

        inline std::string& GetName() { return DebugName; }

    private:
        std::string DebugName;
    };
}
