#include "EditorCamera.h"

#include "glm/gtx/quaternion.hpp"
#include "Input/Input.h"
#include "Input/InputCode.h"
#include "Event/EventDispatcher.h"

ReEngine::EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
{
    UpdateView();
}

void ReEngine::EditorCamera::OnUpdate(Timestep ts)
{
    if (Input::IsKeyPressed(static_cast<uint16_t>(Key::LeftAlt)))
    {
        const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
        glm::vec2 delta = (mouse - mInitialMousePosition) * 0.003f;
        mInitialMousePosition = mouse;

        if (Input::IsMouseButtonPressed(static_cast<uint16_t>(Mouse::ButtonMiddle)))
            MousePan(delta);
        else if (Input::IsMouseButtonPressed(static_cast<uint16_t>(Mouse::ButtonLeft)))
            MouseRotate(delta);
        else if (Input::IsMouseButtonPressed(static_cast<uint16_t>(Mouse::ButtonRight)))
            MouseZoom(delta.y);
    }
    else if (Input::IsMouseButtonPressed(static_cast<uint16_t>(Mouse::ButtonRight)))
    {
        glm::vec2 deltaMove = { 0.0f, 0.0f };

        if (Input::IsKeyPressed(static_cast<uint16_t>(RE_KEY_A)))
            mFocalPoint += GetRightDirection() * mCameraSpeed * 0.05f;
        if (Input::IsKeyPressed(static_cast<uint16_t>(RE_KEY_D)))
            mFocalPoint -= GetRightDirection() * mCameraSpeed * 0.05f;
        if (Input::IsKeyPressed(static_cast<uint16_t>(RE_KEY_W)))
            mFocalPoint += GetForwardDirection() * mCameraSpeed * 0.05f;
        if (Input::IsKeyPressed(static_cast<uint16_t>(RE_KEY_S)))
            mFocalPoint -= GetForwardDirection() * mCameraSpeed * 0.05f;
        if (Input::IsKeyPressed(static_cast<uint16_t>(RE_KEY_Q)))
            mFocalPoint -= glm::vec3(0.0f, 1.0f, 0.0f) * mCameraSpeed * 0.05f;
        if (Input::IsKeyPressed(static_cast<uint16_t>(RE_KEY_E)))
            mFocalPoint += glm::vec3(0.0f, 1.0f, 0.0f) * mCameraSpeed * 0.05f;

        const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
        glm::vec2 deltaRotate = (mouse - mInitialMousePosition) * 0.003f;
        
        if (bInit)
            deltaRotate = { 0.0f, 0.0f };
        mInitialMousePosition = mouse;

        if (Input::IsMouseButtonPressed(static_cast<uint16_t>(Mouse::ButtonRight)))
        {
            bInit = false;
            MouseRotate(deltaRotate);
        }
    }
    else if (Input::IsMouseButtonReleased(static_cast<uint16_t>(Mouse::ButtonRight)))
    {
        bInit = true;
    }

    UpdateView();
    UpdateProjection();
}

void ReEngine::EditorCamera::OnEvent(Ref<Event> e)
{
    EventDispatcher dispatcher(e);
    dispatcher.DispatchEvent<MouseScrollEvent>([&](Ref<MouseScrollEvent> e)->bool
    {
        return EditorCamera::OnMouseScroll(e);
    });
}

glm::vec3 ReEngine::EditorCamera::GetUpDirection() const
{ 
    return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::vec3 ReEngine::EditorCamera::GetRightDirection() const
{
    return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
}

glm::vec3 ReEngine::EditorCamera::GetForwardDirection() const
{
    return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
}

glm::quat ReEngine::EditorCamera::GetOrientation() const
{
    return glm::quat(glm::vec3(-mPitch, -mYaw, 0.0f));
}

void ReEngine::EditorCamera::SetCenter(const glm::vec3& center)
{
    mFocalPoint = center;
    mDistance = 10.0f;
    UpdateView();
}

void ReEngine::EditorCamera::UpdateProjection()
{
    mAspectRatio = mViewportWidth / mViewportHeight;
    mProjection = glm::perspectiveLH_ZO(glm::radians(mFOV), mAspectRatio, mNearClip, mFarClip);
}

void ReEngine::EditorCamera::UpdateView()
{
    mPosition = CalculatePosition();

    glm::quat orientation = GetOrientation();
    mViewMatrix = glm::lookAtLH(mPosition,mFocalPoint,glm::vec3(0.0f,1.0,0.0f));
}

bool ReEngine::EditorCamera::OnMouseScroll(Ref<MouseScrollEvent>& e)
{
    float delta = e->GetYOffset() * 0.1f;
    MouseZoom(delta);
    UpdateView();
    return false;
}

void ReEngine::EditorCamera::MousePan(const glm::vec2& delta)
{
    auto [xSpeed, ySpeed] = PanSpeed();
    mFocalPoint += -GetRightDirection() * delta.x * xSpeed * mDistance;
    mFocalPoint += GetUpDirection() * delta.y * ySpeed * mDistance;
}

void ReEngine::EditorCamera::MouseRotate(const glm::vec2& delta)
{
    float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
    mYaw -= yawSign * delta.x * RotationSpeed();
    mPitch += delta.y * RotationSpeed();
}

void ReEngine::EditorCamera::MouseZoom(float delta)
{
    mDistance -= delta * ZoomSpeed();
    if (mDistance < 1.0f)
    {
        mFocalPoint += GetForwardDirection();
        mDistance = 1.0f;
    }
}

glm::vec3 ReEngine::EditorCamera::CalculatePosition() const
{
    return mFocalPoint - GetForwardDirection() * mDistance;
}

std::pair<float, float> ReEngine::EditorCamera::PanSpeed() const
{
    float x = std::min(mViewportWidth / 1000.0f, 2.4f); // max = 2.4f
    float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

    float y = std::min(mViewportHeight / 1000.0f, 2.4f); // max = 2.4f
    float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

    return { xFactor, yFactor };
}

float ReEngine::EditorCamera::RotationSpeed() const
{
    return 0.8f;
}

float ReEngine::EditorCamera::ZoomSpeed() const
{
    float distance = mDistance * 0.2f;
    distance = std::max(distance, 0.0f);
    float speed = distance * distance;
    speed = std::min(speed, 100.0f); // max speed = 100
    return speed;
}
