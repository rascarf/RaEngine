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
        virtual void OnUpdate(Timestep ts) {}
        virtual void OnEvent(std::shared_ptr<Event> e) {}

        virtual void OnUIRender(Timestep ts){} 

        inline std::string& GetName() { return DebugName; }

    private:
        std::string DebugName;
    };
}

