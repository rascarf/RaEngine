#include "OrthogaraphicCameraController.h"

#include "Event/EventDispatcher.h"
#include "Input/Input.h"
#include "Input/InputCode.h"



namespace ReEngine
{
    OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
        :mAspectRatio(aspectRatio), mCamera(-mAspectRatio * mZoomLevel, mAspectRatio * mZoomLevel, -mZoomLevel, mZoomLevel), mRotation(rotation)
    {
        
    }

    void OrthographicCameraController::OnUpdate(Timestep ts)
    {
        if (Input::IsKeyPressed(static_cast<int16_t>(RE_KEY_A)))
        {
            m_CameraPosition.x -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
            m_CameraPosition.y -= sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
        }
        else if (Input::IsKeyPressed(static_cast<int16_t>(RE_KEY_D)))
        {
            m_CameraPosition.x += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
            m_CameraPosition.y += sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
        }

        if (Input::IsKeyPressed(static_cast<int16_t>(RE_KEY_W)))
        {
            m_CameraPosition.x += -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
            m_CameraPosition.y += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
        }
        else if (Input::IsKeyPressed(static_cast<int16_t>(RE_KEY_S)))
        {
            m_CameraPosition.x -= -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
            m_CameraPosition.y -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
        }

        if (mRotation)
        {
            if (Input::IsKeyPressed(static_cast<int16_t>(RE_KEY_Q)))
                m_CameraRotation += m_CameraRotationSpeed * ts;
            if (Input::IsKeyPressed(static_cast<int16_t>(RE_KEY_E)))
                m_CameraRotation -= m_CameraRotationSpeed * ts;

            if (m_CameraRotation > 180.0f)
                m_CameraRotation -= 360.0f;
            else if (m_CameraRotation <= -180.0f)
                m_CameraRotation += 360.0f;

            mCamera.SetRotation(m_CameraRotation);
        }
        
        mCamera.SetPosition(m_CameraPosition);

        m_CameraTranslationSpeed = mZoomLevel;
    }

    void OrthographicCameraController::OnEvent(Ref<Event> e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.DispatchEvent<MouseScrollEvent>(RE_BIND_EVENT_FN(MouseScrollEvent,OnMouseScrolled));
        dispatcher.DispatchEvent<WindowResizeEvent>(RE_BIND_EVENT_FN(WindowResizeEvent,OnWindowResized));
    }

    bool OrthographicCameraController::OnMouseScrolled(Ref<MouseScrollEvent> e)
    {
        mZoomLevel -= e->GetYOffset() * 0.25f;
        mZoomLevel = std::max(mZoomLevel, 0.25f);
        mCamera.SetProjection(-mAspectRatio * mZoomLevel, mAspectRatio * mZoomLevel, -mZoomLevel, mZoomLevel);
        return false;
    }

    bool OrthographicCameraController::OnWindowResized(Ref<WindowResizeEvent> e)
    {
        mAspectRatio = (float)e->GetWidth() / (float)e->GetHeight();
        mCamera.SetProjection(-mAspectRatio * mZoomLevel, mAspectRatio * mZoomLevel, -mZoomLevel, mZoomLevel);
        return false;
    }
}
