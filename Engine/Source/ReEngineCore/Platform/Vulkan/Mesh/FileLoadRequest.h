#pragma once
#include "Core/Core.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanTexture.h"

namespace ReEngine
{
    struct FileLoadCallBackParamStruct
    {
        std::string             FilePath;
        Ref<VulkanTexture>      TextureHanle;
        Ref<VulkanBuffer>       BufferHanle;
    };

    
    using FileLoadCallBack = std::function<void(FileLoadCallBackParamStruct)>;

    uint32_t MemoryAlign(uint32_t Size,uint32_t Alignment);

    inline uint32_t MemoryAlign(uint32_t Size, uint32_t Alignment)
    {
        const uint32_t alignment_mask = Alignment - 1;
        return ( Size + alignment_mask ) & ~alignment_mask;
    }

    struct FileLoadRequest
    {
    public:
        std::string             Path = "";
        Ref<VulkanTexture>      TextureHanle = nullptr;
        Ref<VulkanBuffer>       BufferHandle = nullptr;

        FileLoadCallBack        Callback;
    };

    struct UploadRequest
    {
        std::string             Path = "";
        void*                   data = nullptr;
        Ref<VulkanTexture>      Texture = nullptr;
        Ref<VulkanBuffer>       CpuBuffer = nullptr;
        Ref<VulkanBuffer>       GpuBuffer = nullptr;

        FileLoadCallBack          Callback;
    };

    struct FileReadyRequest
    {
    public:
        std::string             Path;
        Ref<VulkanTexture>      TextureHanle = nullptr;
        Ref<VulkanBuffer>       BufferHandle = nullptr;
        FileLoadCallBack        Callback;
    };
}

