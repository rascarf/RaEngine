#pragma once

#include "Component.h"
#include "glm/vec4.hpp"

namespace ReEngine
{
    class SpriteRenderComponent : public ComponentBase
    {
    public:
        SpriteRenderComponent() = default;
        SpriteRenderComponent(const SpriteRenderComponent&) = default;
        SpriteRenderComponent(const glm::vec4& color):Color(color){}
        
    public:
        glm::vec4 Color{1.0f,1.0f,1.0f,1.0f};
    };
}