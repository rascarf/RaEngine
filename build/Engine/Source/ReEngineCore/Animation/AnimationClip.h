#pragma once
#include "AnimationChannel.h"
#include "Core/Core.h"
#include "glm/fwd.hpp"
#include "glm/vec3.hpp"

namespace ReEngine
{
    struct AnimationClip
    {
    public:
        std::string NodeName;
        float Duration;

        AnimationChannel<glm::vec3> Positions;
        AnimationChannel<glm::vec3> Scales;
        AnimationChannel<glm::quat> Rotations;
    };

    struct Animation
    {
        std::string Name;
        float Time = 0.0f;
        float Duration = 0.0f;
        float Speed = 1.0f;

        std::unordered_map<std::string,AnimationClip> Clips;
    };
}

