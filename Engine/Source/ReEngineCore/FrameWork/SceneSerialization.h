#pragma once
#include "Scene.h"

namespace ReEngine
{
    class SceneSerializer
    {
    public:
        SceneSerializer(const Ref<Scene>& scene);

        void Serialize(const std::string& FilePath);
        void SerializeRuntime(const std::string& FilePath);

        bool DeSerialize(const std::string& FilePath);
        bool DeSerializeRuntime(const std::string& FilePath);

    private:
        Ref<Scene> mScene;
    };
    
}
