#pragma once
#include "Core/Core.h"
#include "Renderer/RHI/PixelFormat.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"

#define VULKAN_CPU_ALLOCATOR nullptr

using namespace ReEngine;

template<class T>
static  void ZeroVulkanStruct(T& vkStruct, VkStructureType vkType)
{
    vkStruct.sType = vkType;
    memset(((uint8*)&vkStruct) + sizeof(VkStructureType), 0, sizeof(T) - sizeof(VkStructureType));
}

#define VERIFYVULKANRESULT(VkFunction)				{ const VkResult scopedResult = VkFunction; if (scopedResult != VK_SUCCESS) { RE_CORE_ERROR("VKResult=%d,Function=%s,File=%s,Line=%d", scopedResult, #VkFunction, __FILE__, __LINE__); }}
#define VERIFYVULKANRESULT_EXPANDED(VkFunction)		{ const VkResult scopedResult = VkFunction; if (scopedResult < VK_SUCCESS)  { RE_CORE_ERROR("VKResult=%d,Function=%s,File=%s,Line=%d", scopedResult, #VkFunction, __FILE__, __LINE__); }}

template< class T >
static FORCE_INLINE T Clamp(const T x, const T inMin, const T inMax)
{
    return x < inMin ? inMin : x < inMax ? x : inMax;
}

template<typename T> FORCE_INLINE T AlignUp(T val, T alignment)
{
    return (val + alignment - (T)1) & ~(alignment - (T)1);
}

enum class ResLimit
{
    MAX_TEXTURE_MIP_COUNT = 14,
    MaxImmutableSamplers = 2,
    MaxVertexElementCount = 16,
    MaxVertexElementCount_NumBits = 4,
    MaxSimultaneousRenderTargets = 8,
    MaxSimultaneousRenderTargets_NumBits = 3,
    ShaderArrayElementAlignBytes = 16,
    MaxSimultaneousUAVs = 8
};

enum class ImageLayoutBarrier
{
    Undefined,
    TransferDest,
    ColorAttachment,
    DepthStencilAttachment,
    TransferSource,
    Present,
    PixelShaderRead,
    PixelDepthStencilRead,
    ComputeGeneralRW,
    PixelGeneralRW,
};

enum class VertexAttribute
{
    VA_None = 0,
    VA_Position,
    VA_UV0,
    VA_UV1,
    VA_Normal,
    VA_Tangent,
    VA_Color,
    VA_SkinWeight,
    VA_SkinIndex,
    VA_SkinPack,
    VA_InstanceFloat1,
    VA_InstanceFloat2,
    VA_InstanceFloat3,
    VA_InstanceFloat4,
    VA_Custom0,
    VA_Custom1,
    VA_Custom2,
    VA_Custom3,
    VA_Count,
};

enum class VertexElementType
{
    VET_None,
    VET_Float1,
    VET_Float2,
    VET_Float3,
    VET_Float4,
    VET_PackedNormal,
    VET_UByte4,
    VET_UByte4N,
    VET_Color,
    VET_Short2,
    VET_Short4,
    VET_Short2N,
    VET_Half2,
    VET_Half4,
    VET_Short4N,
    VET_UShort2,
    VET_UShort4,
    VET_UShort2N,
    VET_UShort4N,
    VET_URGB10A2N,
    VET_MAX,
    
    VET_NumBits = 5,
};

FORCE_INLINE VertexAttribute StringToVertexAttribute(const char* name)
{
    if (strcmp(name, "inPosition") == 0) {
        return VertexAttribute::VA_Position;
    }
    else if (strcmp(name, "inUV0") == 0) {
        return VertexAttribute::VA_UV0;
    }
    else if (strcmp(name, "inUV1") == 0) {
        return VertexAttribute::VA_UV1;
    }
    else if (strcmp(name, "inNormal") == 0) {
        return VertexAttribute::VA_Normal;
    }
    else if (strcmp(name, "inTangent") == 0) {
        return VertexAttribute::VA_Tangent;
    }
    else if (strcmp(name, "inColor") == 0) {
        return VertexAttribute::VA_Color;
    }
    else if (strcmp(name, "inSkinWeight") == 0) {
        return VertexAttribute::VA_SkinWeight;
    }
    else if (strcmp(name, "inSkinIndex") == 0) {
        return VertexAttribute::VA_SkinIndex;
    }
    else if (strcmp(name, "inSkinPack") == 0) {
        return VertexAttribute::VA_SkinPack;
    }
    else if (strcmp(name, "inCustom0") == 0) {
        return VertexAttribute::VA_Custom0;
    }
    else if (strcmp(name, "inCustom1") == 0) {
        return VertexAttribute::VA_Custom1;
    }
    else if (strcmp(name, "inCustom2") == 0) {
        return VertexAttribute::VA_Custom2;
    }
    else if (strcmp(name, "inCustom3") == 0) {
        return VertexAttribute::VA_Custom3;
    }
	
    return VertexAttribute::VA_None;
}

    FORCE_INLINE VkPipelineStageFlags GetImageBarrierFlags(ImageLayoutBarrier target, VkAccessFlags& accessFlags, VkImageLayout& layout)
    {
        VkPipelineStageFlags stageFlags = (VkPipelineStageFlags)0;
        switch (target)
        {
            case ImageLayoutBarrier::Undefined:
            {
                accessFlags = 0;
                stageFlags  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                layout      = VK_IMAGE_LAYOUT_UNDEFINED;
                break;
            }

            case ImageLayoutBarrier::TransferDest:
            {
                accessFlags = VK_ACCESS_TRANSFER_WRITE_BIT;
                stageFlags  = VK_PIPELINE_STAGE_TRANSFER_BIT;
                layout      = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                break;
            }

            case ImageLayoutBarrier::ColorAttachment:
            {
                accessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                stageFlags  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                break;
            }

            case ImageLayoutBarrier::DepthStencilAttachment:
            {
                accessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                stageFlags  = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                layout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                break;
            }

            case ImageLayoutBarrier::TransferSource:
            {
                accessFlags = VK_ACCESS_TRANSFER_READ_BIT;
                stageFlags  = VK_PIPELINE_STAGE_TRANSFER_BIT;
                layout      = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                break;
            }

            case ImageLayoutBarrier::Present:
            {
                accessFlags = 0;
                stageFlags  = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                layout      = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                break;
            }

            case ImageLayoutBarrier::PixelShaderRead:
            {
                accessFlags = VK_ACCESS_SHADER_READ_BIT;
                stageFlags  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                layout      = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                break;
            }

            case ImageLayoutBarrier::PixelDepthStencilRead:
            {
                accessFlags = VK_ACCESS_SHADER_READ_BIT;
                stageFlags  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                layout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                break;
            }

            case ImageLayoutBarrier::ComputeGeneralRW:
            {
                accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                stageFlags  = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                layout      = VK_IMAGE_LAYOUT_GENERAL;
                break;
            }

            case ImageLayoutBarrier::PixelGeneralRW:
            {
                accessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
                stageFlags  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                layout      = VK_IMAGE_LAYOUT_GENERAL;
                break;
            }

            default:
            {
                RE_CORE_ERROR("Unknown ImageLayoutBarrier %d", (int32)target);
                break;
            }
        }

        return stageFlags;
    }

FORCE_INLINE VkImageLayout GetImageLayout(ImageLayoutBarrier target)
    {
        VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

        switch (target)
        {
            case ImageLayoutBarrier::Undefined:
            {
                layout = VK_IMAGE_LAYOUT_UNDEFINED;
                break;
            }

            case ImageLayoutBarrier::TransferDest:
            {
                layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                break;
            }

            case ImageLayoutBarrier::ColorAttachment:
            {
                layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                break;
            }

            case ImageLayoutBarrier::DepthStencilAttachment:
            {
                layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                break;
            }

            case ImageLayoutBarrier::TransferSource:
            {
                layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                break;
            }

            case ImageLayoutBarrier::Present:
            {
                layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                break;
            }

            case ImageLayoutBarrier::PixelShaderRead:
            {
                layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                break;
            }

            case ImageLayoutBarrier::PixelDepthStencilRead:
            {
                layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                break;
            }

            case ImageLayoutBarrier::ComputeGeneralRW:
            {
                layout = VK_IMAGE_LAYOUT_GENERAL;
                break;
            }

            case ImageLayoutBarrier::PixelGeneralRW:
            {
                layout = VK_IMAGE_LAYOUT_GENERAL;
                break;
            }

            default:
            {
                RE_CORE_ERROR("Unknown ImageLayoutBarrier %d", (int32)target);
                break;
            }
        }

        return layout;
    }