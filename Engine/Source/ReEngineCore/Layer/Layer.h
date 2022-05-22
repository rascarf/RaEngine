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
        virtual void OnEvent(Event* e) {}

        inline std::string& GetName() { return DebugName; }

    private:
        std::string DebugName;
    };
}

