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

        void GetKeys(std::vector<float>& Keys)
        {
            for(int32 i =0 ; i < Positions.Keys.size() ; ++i)
            {
                if(Keys.back() < Positions.Keys[i])
                {
                    Keys.push_back(Positions.Keys[i]);
                }
            }

            for(int32 i =0 ; i < Scales.Keys.size() ; ++i)
            {
                if(Keys.back() < Scales.Keys[i])
                {
                    Keys.push_back(Scales.Keys[i]);
                }
            }

            for(int32 i =0 ; i < Rotations.Keys.size() ; ++i)
            {
                if(Keys.back() < Rotations.Keys[i])
                {
                    Keys.push_back(Rotations.Keys[i]);
                }
            }
        }
    };

    struct Animation
    {
        std::string Name;
        float Time = 0.0f;
        float Duration = 0.0f;
        float Speed = 1.0f;

        std::unordered_map<std::string,AnimationClip> Clips;

    public:
        std::vector<float> GetKeys()
        {
            std::vector<float> Keys;
            Keys.push_back(0);

            for(auto& ClipPair : Clips)
            {
                auto Clip = ClipPair.second;
                Clip.GetKeys(Keys);
            }

            return Keys;
        }
    };
}

