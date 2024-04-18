#pragma once
#include "Core/Core.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanTexture.h"

namespace ReEngine
{
    struct FileLoadRequest
    {
    public:
        char                    Path[512];
        Ref<VulkanTexture>      TextureHanle = nullptr;
        Ref<VulkanBuffer>       BufferHandle = nullptr;
    };

    struct UploadRequest
    {
        void*                   data = nullptr;
        Ref<VulkanTexture>      Texture = nullptr;
        Ref<VulkanBuffer>       CpuBuffer = nullptr;
        Ref<VulkanBuffer>       GpuBuffer = nullptr;
    };
}

