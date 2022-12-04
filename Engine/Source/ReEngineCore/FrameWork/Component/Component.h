#pragma once
#include "Core/Core.h"
#include "Core/PCH.h"
#include <string>

namespace ReEngine
{
    class ComponentBase
    {
    public:
        ComponentBase() = default;
        virtual ~ComponentBase(){}
    };

    class TagComponent : public ComponentBase
    {
    public:
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent& other) = default;
        TagComponent(const std::string& tag) : Tag(tag) {}
    };

    template<class T>
    concept Component = std::is_base_of_v<ComponentBase, T>;
}


