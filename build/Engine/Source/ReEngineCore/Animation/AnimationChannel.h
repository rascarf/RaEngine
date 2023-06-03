#pragma once
#include "Core/Core.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"


namespace ReEngine
{
    template<class ValueType>
    struct AnimationChannel
    {
    public:

        std::vector<float> Keys;
        std::vector<ValueType> Values;

        void GetValue(float key,ValueType& OutPrevValue,ValueType& OutNextValue,float& outAlpha)
        {
            outAlpha = 0.0f;

            if(Keys.size() == 0)
            {
                return;
            }

            if(key <= Keys.front())
            {
                OutPrevValue = Values.front();
                OutNextValue = Values.front();
                outAlpha     = 0.0f;
                return;
            }

            if(key >= Keys.back())
            {
                OutPrevValue = Values.back();
                OutNextValue = Values.back();
                outAlpha     = 1.0f;
                return;
            }

            int32 frameIndex = 0;
            for (int32 i = 0; i < Keys.size() - 1; ++i)
            {
                if (key <= Keys[i + 1])
                {
                    frameIndex = i;
                    break;
                }
            }

            OutPrevValue = Values[frameIndex + 0];
            OutNextValue = Values[frameIndex + 1];

            float prevKey = Keys[frameIndex + 0];
            float nextKey = Keys[frameIndex + 1];
            outAlpha      = (key - prevKey) / (nextKey - prevKey);
        }
    };

   
}

