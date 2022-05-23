#pragma once
#include "Layer/Layer.h"

namespace ReEngine
{
    class ImGuiLayer :public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer();

        void OnUpdate() override;
        void OnEvent(Event* e) override;

        void OnAttach() override;
        virtual void OnDetach() override;
    private:

    };
}


