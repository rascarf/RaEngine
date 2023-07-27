#pragma once
#include "Core/Core.h"
#include "glm/mat4x4.hpp"

namespace ReEngine
{
    struct Bone
    {
        std::string Name;
        int32 Index = -1;
        int32 Parent = -1;

        glm::mat4 InverseBindPose;
        glm::mat4 FinalTransform;
    };
}

