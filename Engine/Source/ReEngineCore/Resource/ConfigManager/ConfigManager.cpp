#include "ConfigManager.h"

namespace ReEngine
{
    ConfigManager::ConfigManager()
    {
        mRootFolder = { RE_XSTRINGIFY_MACRO(ENGINE_ROOT_DIR) };
        mAssetsFolder = mRootFolder / "Assets";
        mShadersFolder = mRootFolder / "Shaders";
        mResourcesFolder = mRootFolder / "Resources";
    }
    
    void ConfigManager::Clear()
    {
        mRootFolder.clear();
        mAssetsFolder.clear();
        mShadersFolder.clear();
        mResourcesFolder.clear();
    }

    const std::filesystem::path& ConfigManager::GetRootFolder() const
    {
        return mRootFolder;
    }

    const std::filesystem::path& ConfigManager::GetAssetsFolder() const
    {
        return mAssetsFolder;
    }

    const std::filesystem::path& ConfigManager::GetShadersFolder() const
    {
        return mShadersFolder;
    }

    const std::filesystem::path& ConfigManager::GetResourcesFolder() const
    {
        return mResourcesFolder;
    }
}
