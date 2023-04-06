#pragma once
#include "Core/MathDefine.h"
#include "glm/glm.hpp"

namespace ReEngine
{
    class BoundingBox
    {
    public:
        glm::vec3 Min;
        glm::vec3 Max;
        glm::vec3 corners[8];

        BoundingBox():Min(MAX_FLT,MAX_FLT,MAX_FLT),Max(MIN_flt,MIN_flt,MIN_flt){}
        
        BoundingBox(const glm::vec3& inMin, const glm::vec3& inMax)
       : Min(inMin)
       , Max(inMax)
        {
            
        }

        void UpdateCorners()
        {
            corners[0] = glm::vec3(Min.x, Min.y, Min.z);
            corners[1] = glm::vec3(Max.x, Min.y, Min.z);
            corners[2] = glm::vec3(Min.x, Max.y, Min.z);
            corners[3] = glm::vec3(Max.x, Max.y, Min.z);

            corners[4] = glm::vec3(Min.x, Min.y, Max.z);
            corners[5] = glm::vec3(Max.x, Min.y, Max.z);
            corners[6] = glm::vec3(Min.x, Max.y, Max.z);
            corners[7] = glm::vec3(Max.x, Max.y, Max.z);
        }
        
    };
}

