#pragma once
#include "Core/PCH.h"
#include "Core/Core.h"

#include "ThirdParty/entt/include/entt.hpp"
namespace ReEngine
{
    class Scene
    {
    public:
        Scene();
        ~Scene();

    private:
        entt::registry mRegistry; //保存Component的Data和ID
    
    };
}

