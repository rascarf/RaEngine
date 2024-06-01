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

        [[nodiscard]] glm::mat4 GetTransform()const
        {
           return LocalMatrix;
        }

        glm::mat4 LocalMatrix = glm::identity<glm::mat4>();
    };
}
