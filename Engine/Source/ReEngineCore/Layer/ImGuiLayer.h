#pragma once
#include "Event/ApplicationEvent.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"
#include "Layer/Layer.h"

namespace ReEngine
{
    class ImGuiLayer :public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnEvent(std::shared_ptr<Event> e) override;
        virtual void OnUpdate() override;
        virtual void Begin() override;
        virtual void End() override;
        virtual void OnUIRender() override;

    private:
        bool OnMouseButtonPressedEvent(std::shared_ptr<MouseButtonPressedEvent> e);
        bool OnMouseButtonReleasedEvent(std::shared_ptr<MouseButtonReleasedEvent> e);
        bool OnMouseMovedEvent(std::shared_ptr<MouseMoveEvent> e);
        bool OnMouseScrolledEvent(std::shared_ptr<MouseScrollEvent> e);
        bool OnKeyPressedEvent(std::shared_ptr<KeyPressedEvent> e);
        bool OnKeyReleasedEvent(std::shared_ptr<KeyReleasedEvent> e);
        bool OnKeyTypedEvent(std::shared_ptr<KeyTypedEvent> e);
        bool OnWindowResizeEvent(std::shared_ptr<WindowResizeEvent> e);

    };
}


