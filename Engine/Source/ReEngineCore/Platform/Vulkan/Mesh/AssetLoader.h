#pragma once
#include <set>

#include "VulkanMesh.h"
#include "Core/Core.h"

namespace ReEngine
{
    struct ModelLoadCallBackParam
    {
        std::string         FilePath;
        Ref<VulkanModel>    ModelHandle;
    };
    
    using ModelLoadCallBack = std::function<void(ModelLoadCallBackParam)>;
    struct ModelLoadRequest
    {
        std::string                      filename;
        std::set<VertexAttribute>        VertexAttributes = {VertexAttribute::VA_Position,VertexAttribute::VA_Normal,VertexAttribute::VA_UV0};
        Ref<VulkanModel>                 ModelHandle = nullptr;
        
        ModelLoadCallBack                CallBack;
    };
    
    class AssetLoader
    {
    public:
    
    };
}


