#pragma once
#include "Core/Core.h"
#include "Core/PCH.h"
#include "glm/glm.hpp"
#include "Core/SIngletonTemplate.h"
#include <filesystem>

namespace ReEngine
{
    class ConfigManager final :public SingletonTemplate<ConfigManager>
    {
    public:
        ConfigManager();
        ConfigManager(const ConfigManager&);
        ConfigManager& operator=(const ConfigManager&);

    public:
        void Clear();
        [[nodiscard]] const std::filesystem::path& GetRootFolder() const;
        [[nodiscard]] const std::filesystem::path& GetAssetsFolder() const;
        [[nodiscard]] const std::filesystem::path& GetShadersFolder() const;
        [[nodiscard]] const std::filesystem::path& GetResourcesFolder() const;
    

    private:
        std::filesystem::path mRootFolder;
        std::filesystem::path mAssetsFolder;
        std::filesystem::path mShadersFolder;
        std::filesystem::path mResourcesFolder;
    };
}
