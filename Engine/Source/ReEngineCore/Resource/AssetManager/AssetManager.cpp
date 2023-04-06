#include "AssetManager.h"

bool ReEngine::AssetManager::ReadFile(const std::string& filepath, uint8*& dataPtr, uint32& dataSize)
{
    auto finalPath = AssetManager::GetFullPath(filepath);
    FILE* file = fopen(finalPath.generic_string().c_str(), "rb");
    if (!file)
    {
        RE_CORE_ERROR("File not found model:{0}", finalPath.generic_string());
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
    
    return true;
}
