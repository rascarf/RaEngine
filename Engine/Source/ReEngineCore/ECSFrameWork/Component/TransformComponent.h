#pragma once
#include "Component.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

namespace ReEngine
{
    class TransformComponent : public ComponentBase
    {
    public:
        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3& InTranslation):Translation(InTranslation){}

        TransformComponent(
        const glm::vec3& InTranslation,
        const glm::vec3& InRotation,
        const glm::vec3 InScale
        ):Translation(InTranslation),Rotation(InRotation),Scale(InScale){}

        [[nodiscard]] glm::mat4 GetTransform()const
        {
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), Rotation.x, { 1, 0, 0 })
                * glm::rotate(glm::mat4(1.0f), Rotation.y, { 0, 1, 0 })
                * glm::rotate(glm::mat4(1.0f), Rotation.z, { 0, 0, 1 });
            
            return glm::translate(glm::mat4(1.0),Translation)* rotation * glm::scale(glm::mat4(1.0f), Scale);
        }

        void SetTranslation(float x, float y, float z) { Translation = {x, y, z}; }

    public:
        glm::vec3 Translation = {0.0f,0.0f,0.0f};
        glm::vec3 Rotation = {0.0f,0.0f,0.0f}; //Euler Angle
        glm::vec3 Scale = {1.0f,1.0f,1.0f};
    };
}
