#pragma once
#include "Core/Core.h"
#include "Core/PCH.h"
#include "Resource/ConfigManager/ConfigManager.h"
#include "Core/SIngletonTemplate.h"
#include <filesystem>

namespace ReEngine
{
    class AssetManager :public SingletonTemplate<AssetManager>
    {
    public:
        [[nodiscard]] static std::filesystem::path GetFullPath(const std::string& RelativePath)
        {
            return ConfigManager::GetInstance().GetRootFolder() / RelativePath;
        }
    
    };
}
