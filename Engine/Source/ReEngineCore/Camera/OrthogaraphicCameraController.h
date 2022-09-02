#include "Core/PCH.h"
#include "Core/Timestep.h"
#include "Event/ApplicationEvent.h"
#include "Event/MouseEvent.h"
#include "OrthographicCamera.h"

namespace ReEngine
{
    class OrthographicCameraController
    {
    public:
        OrthographicCameraController(float aspectRatio, bool rotation = true);

        void OnUpdate(Timestep ts);
        void OnEvent(Ref<Event> e);

        [[nodiscard]] const OrthographicCamera& GetCamera()const {return mCamera;}
        OrthographicCamera& GetCamera() { return mCamera; }

        float GetZoomLevel() const { return mZoomLevel; }
        void SetZoomLevel(float zoomLevel) { mZoomLevel = zoomLevel; }

    private:
        bool OnMouseScrolled(Ref<MouseScrollEvent> e);
        bool OnWindowResized(Ref<WindowResizeEvent> e);

    private:
        float mAspectRatio;
        float mZoomLevel = 1.0f;
        bool mRotation;
        OrthographicCamera mCamera;

        glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
        float m_CameraRotation = 0.0f; //In degrees, in the anti-clockwise direction
        float m_CameraTranslationSpeed = 5.0f, m_CameraRotationSpeed = 180.0f;
    };
}
