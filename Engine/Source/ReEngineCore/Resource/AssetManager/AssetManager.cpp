#include "AssetManager.h"

bool ReEngine::AssetManager::ReadFile(const std::string& filepath, uint8*& dataPtr, uint32& dataSize)
{
    auto finalPath = AssetManager::GetFullPath(filepath);

#if PLATFORM_ANDROID

    AAsset* asset = AAssetManager_open(g_AndroidApp->activity->assetManager, finalPath.c_str(), AASSET_MODE_STREAMING);
    dataSize = AAsset_getLength(asset);
    dataPtr = new uint8[dataSize];
    AAsset_read(asset, dataPtr, dataSize);
    AAsset_close(asset);

#else

    FILE* file = fopen((char const*)finalPath.c_str(), "rb");
    if (!file)
    {
        RE_CORE_ERROR("File not found :{0}", filepath.c_str());
        return false;
    }

    fseek(file, 0, SEEK_END);
    dataSize = (uint32)ftell(file);
    fseek(file, 0, SEEK_SET);

    if (dataSize <= 0)
    {
        fclose(file);
        RE_CORE_ERROR("File has no data :{0}", filepath.c_str());
        return false;
    }

    dataPtr = new uint8[dataSize];
    fread(dataPtr, 1, dataSize, file);
    fclose(file);

#endif

    return true;
}
