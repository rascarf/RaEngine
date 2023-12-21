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

    dataPtr = new uint8[dataSize + 1];

    // 逆天windows会给文件结尾塞东西，塞进来的这部分东西不希望是拿到的
    size_t BytesRead = fread(dataPtr, 1, dataSize, file);

    dataPtr[BytesRead] = 0;
    dataSize = BytesRead;
    
    fclose(file);
    
    return true;
}
