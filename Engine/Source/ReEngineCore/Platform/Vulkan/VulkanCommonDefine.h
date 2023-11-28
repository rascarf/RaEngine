#pragma once
#include "Core/Core.h"
#include "Renderer/RHI/PixelFormat.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include "volk.h"

#define HWRAYTRACING false

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

template <class T>
constexpr T align_up(T x, size_t a) noexcept {
    return T((x + (T(a) - 1)) & ~T(a - 1));
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

FORCE_INLINE int32 VertexAttributeToSize(VertexAttribute attribute)
{
    // count * sizeof(float)
    if (attribute == VertexAttribute::VA_Position)
    {
        return 3 * sizeof(float);
    }
    else if (attribute == VertexAttribute::VA_UV0)
    {
        return 2 * sizeof(float);
    }
    else if (attribute == VertexAttribute::VA_UV1)
    {
        return 2 * sizeof(float);
    }
    else if (attribute == VertexAttribute::VA_Normal)
    {
        return 3 * sizeof(float);
    }
    else if (attribute == VertexAttribute::VA_Tangent)
    {
        return 4 * sizeof(float);
    }
    else if (attribute == VertexAttribute::VA_Color)
    {
        return 3 * sizeof(float);
    }
    else if (attribute == VertexAttribute::VA_SkinWeight)
    {
        return 4 * sizeof(float);
    }
    else if (attribute == VertexAttribute::VA_SkinIndex)
    {
        return 4 * sizeof(float);
    }
    else if (attribute == VertexAttribute::VA_SkinPack)
    {
        return 3 * sizeof(float);
    }
    else if (attribute == VertexAttribute::VA_Custom0 ||
             attribute == VertexAttribute::VA_Custom1 ||
             attribute == VertexAttribute::VA_Custom2 ||
             attribute == VertexAttribute::VA_Custom3
    )
    {
        return 4 * sizeof(float);
    }
    else if (attribute == VertexAttribute::VA_InstanceFloat1)
    {
        return 1 * sizeof(float);
    }
    else if (attribute == VertexAttribute::VA_InstanceFloat2)
    {
        return 2 * sizeof(float);
    }
    else if (attribute == VertexAttribute::VA_InstanceFloat3)
    {
        return 3 * sizeof(float);
    }
    else if (attribute == VertexAttribute::VA_InstanceFloat4)
    {
        return 4 * sizeof(float);
    }

    return 0;
}


FORCE_INLINE VkFormat VertexAttributeToVkFormat(VertexAttribute attribute)
{
    VkFormat format = VK_FORMAT_R32G32B32_SFLOAT;
    if (attribute == VertexAttribute::VA_Position)
    {
        format = VK_FORMAT_R32G32B32_SFLOAT;
    }
    else if (attribute == VertexAttribute::VA_UV0)
    {
        format = VK_FORMAT_R32G32_SFLOAT;
    }
    else if (attribute == VertexAttribute::VA_UV1)
    {
        format = VK_FORMAT_R32G32_SFLOAT;
    }
    else if (attribute == VertexAttribute::VA_Normal)
    {
        format = VK_FORMAT_R32G32B32_SFLOAT;
    }
    else if (attribute == VertexAttribute::VA_Tangent)
    {
        format = VK_FORMAT_R32G32B32A32_SFLOAT;
    }
    else if (attribute == VertexAttribute::VA_Color)
    {
        format = VK_FORMAT_R32G32B32_SFLOAT;
    }
    else if (attribute == VertexAttribute::VA_SkinPack)
    {
        format = VK_FORMAT_R32G32B32_SFLOAT;
    }
    else if (attribute == VertexAttribute::VA_SkinWeight)
    {
        format = VK_FORMAT_R32G32B32A32_SFLOAT;
    }
    else if (attribute == VertexAttribute::VA_SkinIndex)
    {
        format = VK_FORMAT_R32G32B32A32_SFLOAT;
    }
    else if (attribute == VertexAttribute::VA_Custom0 ||
             attribute == VertexAttribute::VA_Custom1 ||
             attribute == VertexAttribute::VA_Custom2 ||
             attribute == VertexAttribute::VA_Custom3
    )
    {
        format = VK_FORMAT_R32G32B32A32_SFLOAT;
    }
    else if (attribute == VertexAttribute::VA_InstanceFloat1)
    {
        format = VK_FORMAT_R32_SFLOAT;
    }
    else if (attribute == VertexAttribute::VA_InstanceFloat2)
    {
        format = VK_FORMAT_R32G32_SFLOAT;
    }
    else if (attribute == VertexAttribute::VA_InstanceFloat3)
    {
        format = VK_FORMAT_R32G32B32_SFLOAT;
    }
    else if (attribute == VertexAttribute::VA_InstanceFloat4)
    {
        format = VK_FORMAT_R32G32B32A32_SFLOAT;
    }

    return format;
}

static VkFormat util_string_to_vk_format( cstring format ) {
    if ( strcmp( format, "VK_FORMAT_R4G4_UNORM_PACK8" ) == 0 ) {
        return VK_FORMAT_R4G4_UNORM_PACK8;
    }
    if ( strcmp( format, "VK_FORMAT_R4G4B4A4_UNORM_PACK16" ) == 0 ) {
        return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_B4G4R4A4_UNORM_PACK16" ) == 0 ) {
        return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_R5G6B5_UNORM_PACK16" ) == 0 ) {
        return VK_FORMAT_R5G6B5_UNORM_PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_B5G6R5_UNORM_PACK16" ) == 0 ) {
        return VK_FORMAT_B5G6R5_UNORM_PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_R5G5B5A1_UNORM_PACK16" ) == 0 ) {
        return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_B5G5R5A1_UNORM_PACK16" ) == 0 ) {
        return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_A1R5G5B5_UNORM_PACK16" ) == 0 ) {
        return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_R8_UNORM" ) == 0 ) {
        return VK_FORMAT_R8_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_R8_SNORM" ) == 0 ) {
        return VK_FORMAT_R8_SNORM;
    }
    if ( strcmp( format, "VK_FORMAT_R8_USCALED" ) == 0 ) {
        return VK_FORMAT_R8_USCALED;
    }
    if ( strcmp( format, "VK_FORMAT_R8_SSCALED" ) == 0 ) {
        return VK_FORMAT_R8_SSCALED;
    }
    if ( strcmp( format, "VK_FORMAT_R8_UINT" ) == 0 ) {
        return VK_FORMAT_R8_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_R8_SINT" ) == 0 ) {
        return VK_FORMAT_R8_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_R8_SRGB" ) == 0 ) {
        return VK_FORMAT_R8_SRGB;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8_UNORM" ) == 0 ) {
        return VK_FORMAT_R8G8_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8_SNORM" ) == 0 ) {
        return VK_FORMAT_R8G8_SNORM;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8_USCALED" ) == 0 ) {
        return VK_FORMAT_R8G8_USCALED;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8_SSCALED" ) == 0 ) {
        return VK_FORMAT_R8G8_SSCALED;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8_UINT" ) == 0 ) {
        return VK_FORMAT_R8G8_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8_SINT" ) == 0 ) {
        return VK_FORMAT_R8G8_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8_SRGB" ) == 0 ) {
        return VK_FORMAT_R8G8_SRGB;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8B8_UNORM" ) == 0 ) {
        return VK_FORMAT_R8G8B8_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8B8_SNORM" ) == 0 ) {
        return VK_FORMAT_R8G8B8_SNORM;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8B8_USCALED" ) == 0 ) {
        return VK_FORMAT_R8G8B8_USCALED;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8B8_SSCALED" ) == 0 ) {
        return VK_FORMAT_R8G8B8_SSCALED;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8B8_UINT" ) == 0 ) {
        return VK_FORMAT_R8G8B8_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8B8_SINT" ) == 0 ) {
        return VK_FORMAT_R8G8B8_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8B8_SRGB" ) == 0 ) {
        return VK_FORMAT_R8G8B8_SRGB;
    }
    if ( strcmp( format, "VK_FORMAT_B8G8R8_UNORM" ) == 0 ) {
        return VK_FORMAT_B8G8R8_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_B8G8R8_SNORM" ) == 0 ) {
        return VK_FORMAT_B8G8R8_SNORM;
    }
    if ( strcmp( format, "VK_FORMAT_B8G8R8_USCALED" ) == 0 ) {
        return VK_FORMAT_B8G8R8_USCALED;
    }
    if ( strcmp( format, "VK_FORMAT_B8G8R8_SSCALED" ) == 0 ) {
        return VK_FORMAT_B8G8R8_SSCALED;
    }
    if ( strcmp( format, "VK_FORMAT_B8G8R8_UINT" ) == 0 ) {
        return VK_FORMAT_B8G8R8_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_B8G8R8_SINT" ) == 0 ) {
        return VK_FORMAT_B8G8R8_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_B8G8R8_SRGB" ) == 0 ) {
        return VK_FORMAT_B8G8R8_SRGB;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8B8A8_UNORM" ) == 0 ) {
        return VK_FORMAT_R8G8B8A8_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8B8A8_SNORM" ) == 0 ) {
        return VK_FORMAT_R8G8B8A8_SNORM;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8B8A8_USCALED" ) == 0 ) {
        return VK_FORMAT_R8G8B8A8_USCALED;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8B8A8_SSCALED" ) == 0 ) {
        return VK_FORMAT_R8G8B8A8_SSCALED;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8B8A8_UINT" ) == 0 ) {
        return VK_FORMAT_R8G8B8A8_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8B8A8_SINT" ) == 0 ) {
        return VK_FORMAT_R8G8B8A8_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_R8G8B8A8_SRGB" ) == 0 ) {
        return VK_FORMAT_R8G8B8A8_SRGB;
    }
    if ( strcmp( format, "VK_FORMAT_B8G8R8A8_UNORM" ) == 0 ) {
        return VK_FORMAT_B8G8R8A8_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_B8G8R8A8_SNORM" ) == 0 ) {
        return VK_FORMAT_B8G8R8A8_SNORM;
    }
    if ( strcmp( format, "VK_FORMAT_B8G8R8A8_USCALED" ) == 0 ) {
        return VK_FORMAT_B8G8R8A8_USCALED;
    }
    if ( strcmp( format, "VK_FORMAT_B8G8R8A8_SSCALED" ) == 0 ) {
        return VK_FORMAT_B8G8R8A8_SSCALED;
    }
    if ( strcmp( format, "VK_FORMAT_B8G8R8A8_UINT" ) == 0 ) {
        return VK_FORMAT_B8G8R8A8_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_B8G8R8A8_SINT" ) == 0 ) {
        return VK_FORMAT_B8G8R8A8_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_B8G8R8A8_SRGB" ) == 0 ) {
        return VK_FORMAT_B8G8R8A8_SRGB;
    }
    if ( strcmp( format, "VK_FORMAT_A8B8G8R8_UNORM_PACK32" ) == 0 ) {
        return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A8B8G8R8_SNORM_PACK32" ) == 0 ) {
        return VK_FORMAT_A8B8G8R8_SNORM_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A8B8G8R8_USCALED_PACK32" ) == 0 ) {
        return VK_FORMAT_A8B8G8R8_USCALED_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A8B8G8R8_SSCALED_PACK32" ) == 0 ) {
        return VK_FORMAT_A8B8G8R8_SSCALED_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A8B8G8R8_UINT_PACK32" ) == 0 ) {
        return VK_FORMAT_A8B8G8R8_UINT_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A8B8G8R8_SINT_PACK32" ) == 0 ) {
        return VK_FORMAT_A8B8G8R8_SINT_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A8B8G8R8_SRGB_PACK32" ) == 0 ) {
        return VK_FORMAT_A8B8G8R8_SRGB_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A2R10G10B10_UNORM_PACK32" ) == 0 ) {
        return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A2R10G10B10_SNORM_PACK32" ) == 0 ) {
        return VK_FORMAT_A2R10G10B10_SNORM_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A2R10G10B10_USCALED_PACK32" ) == 0 ) {
        return VK_FORMAT_A2R10G10B10_USCALED_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A2R10G10B10_SSCALED_PACK32" ) == 0 ) {
        return VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A2R10G10B10_UINT_PACK32" ) == 0 ) {
        return VK_FORMAT_A2R10G10B10_UINT_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A2R10G10B10_SINT_PACK32" ) == 0 ) {
        return VK_FORMAT_A2R10G10B10_SINT_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A2B10G10R10_UNORM_PACK32" ) == 0 ) {
        return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A2B10G10R10_SNORM_PACK32" ) == 0 ) {
        return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A2B10G10R10_USCALED_PACK32" ) == 0 ) {
        return VK_FORMAT_A2B10G10R10_USCALED_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A2B10G10R10_SSCALED_PACK32" ) == 0 ) {
        return VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A2B10G10R10_UINT_PACK32" ) == 0 ) {
        return VK_FORMAT_A2B10G10R10_UINT_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_A2B10G10R10_SINT_PACK32" ) == 0 ) {
        return VK_FORMAT_A2B10G10R10_SINT_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_R16_UNORM" ) == 0 ) {
        return VK_FORMAT_R16_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_R16_SNORM" ) == 0 ) {
        return VK_FORMAT_R16_SNORM;
    }
    if ( strcmp( format, "VK_FORMAT_R16_USCALED" ) == 0 ) {
        return VK_FORMAT_R16_USCALED;
    }
    if ( strcmp( format, "VK_FORMAT_R16_SSCALED" ) == 0 ) {
        return VK_FORMAT_R16_SSCALED;
    }
    if ( strcmp( format, "VK_FORMAT_R16_UINT" ) == 0 ) {
        return VK_FORMAT_R16_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_R16_SINT" ) == 0 ) {
        return VK_FORMAT_R16_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_R16_SFLOAT" ) == 0 ) {
        return VK_FORMAT_R16_SFLOAT;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16_UNORM" ) == 0 ) {
        return VK_FORMAT_R16G16_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16_SNORM" ) == 0 ) {
        return VK_FORMAT_R16G16_SNORM;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16_USCALED" ) == 0 ) {
        return VK_FORMAT_R16G16_USCALED;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16_SSCALED" ) == 0 ) {
        return VK_FORMAT_R16G16_SSCALED;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16_UINT" ) == 0 ) {
        return VK_FORMAT_R16G16_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16_SINT" ) == 0 ) {
        return VK_FORMAT_R16G16_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16_SFLOAT" ) == 0 ) {
        return VK_FORMAT_R16G16_SFLOAT;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16B16_UNORM" ) == 0 ) {
        return VK_FORMAT_R16G16B16_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16B16_SNORM" ) == 0 ) {
        return VK_FORMAT_R16G16B16_SNORM;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16B16_USCALED" ) == 0 ) {
        return VK_FORMAT_R16G16B16_USCALED;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16B16_SSCALED" ) == 0 ) {
        return VK_FORMAT_R16G16B16_SSCALED;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16B16_UINT" ) == 0 ) {
        return VK_FORMAT_R16G16B16_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16B16_SINT" ) == 0 ) {
        return VK_FORMAT_R16G16B16_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16B16_SFLOAT" ) == 0 ) {
        return VK_FORMAT_R16G16B16_SFLOAT;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16B16A16_UNORM" ) == 0 ) {
        return VK_FORMAT_R16G16B16A16_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16B16A16_SNORM" ) == 0 ) {
        return VK_FORMAT_R16G16B16A16_SNORM;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16B16A16_USCALED" ) == 0 ) {
        return VK_FORMAT_R16G16B16A16_USCALED;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16B16A16_SSCALED" ) == 0 ) {
        return VK_FORMAT_R16G16B16A16_SSCALED;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16B16A16_UINT" ) == 0 ) {
        return VK_FORMAT_R16G16B16A16_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16B16A16_SINT" ) == 0 ) {
        return VK_FORMAT_R16G16B16A16_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_R16G16B16A16_SFLOAT" ) == 0 ) {
        return VK_FORMAT_R16G16B16A16_SFLOAT;
    }
    if ( strcmp( format, "VK_FORMAT_R32_UINT" ) == 0 ) {
        return VK_FORMAT_R32_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_R32_SINT" ) == 0 ) {
        return VK_FORMAT_R32_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_R32_SFLOAT" ) == 0 ) {
        return VK_FORMAT_R32_SFLOAT;
    }
    if ( strcmp( format, "VK_FORMAT_R32G32_UINT" ) == 0 ) {
        return VK_FORMAT_R32G32_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_R32G32_SINT" ) == 0 ) {
        return VK_FORMAT_R32G32_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_R32G32_SFLOAT" ) == 0 ) {
        return VK_FORMAT_R32G32_SFLOAT;
    }
    if ( strcmp( format, "VK_FORMAT_R32G32B32_UINT" ) == 0 ) {
        return VK_FORMAT_R32G32B32_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_R32G32B32_SINT" ) == 0 ) {
        return VK_FORMAT_R32G32B32_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_R32G32B32_SFLOAT" ) == 0 ) {
        return VK_FORMAT_R32G32B32_SFLOAT;
    }
    if ( strcmp( format, "VK_FORMAT_R32G32B32A32_UINT" ) == 0 ) {
        return VK_FORMAT_R32G32B32A32_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_R32G32B32A32_SINT" ) == 0 ) {
        return VK_FORMAT_R32G32B32A32_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_R32G32B32A32_SFLOAT" ) == 0 ) {
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    }
    if ( strcmp( format, "VK_FORMAT_R64_UINT" ) == 0 ) {
        return VK_FORMAT_R64_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_R64_SINT" ) == 0 ) {
        return VK_FORMAT_R64_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_R64_SFLOAT" ) == 0 ) {
        return VK_FORMAT_R64_SFLOAT;
    }
    if ( strcmp( format, "VK_FORMAT_R64G64_UINT" ) == 0 ) {
        return VK_FORMAT_R64G64_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_R64G64_SINT" ) == 0 ) {
        return VK_FORMAT_R64G64_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_R64G64_SFLOAT" ) == 0 ) {
        return VK_FORMAT_R64G64_SFLOAT;
    }
    if ( strcmp( format, "VK_FORMAT_R64G64B64_UINT" ) == 0 ) {
        return VK_FORMAT_R64G64B64_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_R64G64B64_SINT" ) == 0 ) {
        return VK_FORMAT_R64G64B64_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_R64G64B64_SFLOAT" ) == 0 ) {
        return VK_FORMAT_R64G64B64_SFLOAT;
    }
    if ( strcmp( format, "VK_FORMAT_R64G64B64A64_UINT" ) == 0 ) {
        return VK_FORMAT_R64G64B64A64_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_R64G64B64A64_SINT" ) == 0 ) {
        return VK_FORMAT_R64G64B64A64_SINT;
    }
    if ( strcmp( format, "VK_FORMAT_R64G64B64A64_SFLOAT" ) == 0 ) {
        return VK_FORMAT_R64G64B64A64_SFLOAT;
    }
    if ( strcmp( format, "VK_FORMAT_B10G11R11_UFLOAT_PACK32" ) == 0 ) {
        return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32" ) == 0 ) {
        return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_D16_UNORM" ) == 0 ) {
        return VK_FORMAT_D16_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_X8_D24_UNORM_PACK32" ) == 0 ) {
        return VK_FORMAT_X8_D24_UNORM_PACK32;
    }
    if ( strcmp( format, "VK_FORMAT_D32_SFLOAT" ) == 0 ) {
        return VK_FORMAT_D32_SFLOAT;
    }
    if ( strcmp( format, "VK_FORMAT_S8_UINT" ) == 0 ) {
        return VK_FORMAT_S8_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_D16_UNORM_S8_UINT" ) == 0 ) {
        return VK_FORMAT_D16_UNORM_S8_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_D24_UNORM_S8_UINT" ) == 0 ) {
        return VK_FORMAT_D24_UNORM_S8_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_D32_SFLOAT_S8_UINT" ) == 0 ) {
        return VK_FORMAT_D32_SFLOAT_S8_UINT;
    }
    if ( strcmp( format, "VK_FORMAT_BC1_RGB_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_BC1_RGB_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_BC1_RGBA_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_BC1_RGBA_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_BC2_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_BC2_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_BC2_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_BC2_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_BC3_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_BC3_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_BC3_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_BC3_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_BC4_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_BC4_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_BC4_SNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_BC4_SNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_BC5_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_BC5_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_BC5_SNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_BC5_SNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_BC6H_UFLOAT_BLOCK" ) == 0 ) {
        return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_BC6H_SFLOAT_BLOCK" ) == 0 ) {
        return VK_FORMAT_BC6H_SFLOAT_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_BC7_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_BC7_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_BC7_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_BC7_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_EAC_R11_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_EAC_R11_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_EAC_R11_SNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_EAC_R11_SNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_EAC_R11G11_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_EAC_R11G11_SNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_4x4_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_4x4_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_5x4_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_5x4_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_5x5_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_5x5_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_6x5_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_6x5_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_6x6_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_6x6_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_8x5_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_8x5_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_8x6_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_8x6_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_8x8_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_8x8_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_10x5_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_10x5_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_10x6_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_10x6_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_10x8_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_10x8_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_10x10_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_10x10_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_12x10_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_12x10_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_12x12_UNORM_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_12x12_SRGB_BLOCK" ) == 0 ) {
        return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;
    }
    if ( strcmp( format, "VK_FORMAT_G8B8G8R8_422_UNORM" ) == 0 ) {
        return VK_FORMAT_G8B8G8R8_422_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_B8G8R8G8_422_UNORM" ) == 0 ) {
        return VK_FORMAT_B8G8R8G8_422_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM" ) == 0 ) {
        return VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM" ) == 0 ) {
        return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM" ) == 0 ) {
        return VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM" ) == 0 ) {
        return VK_FORMAT_G8_B8R8_2PLANE_422_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM" ) == 0 ) {
        return VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_R10X6_UNORM_PACK16" ) == 0 ) {
        return VK_FORMAT_R10X6_UNORM_PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_R10X6G10X6_UNORM_2PACK16" ) == 0 ) {
        return VK_FORMAT_R10X6G10X6_UNORM_2PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16" ) == 0 ) {
        return VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16" ) == 0 ) {
        return VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16" ) == 0 ) {
        return VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16" ) == 0 ) {
        return VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16" ) == 0 ) {
        return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16" ) == 0 ) {
        return VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16" ) == 0 ) {
        return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16" ) == 0 ) {
        return VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_R12X4_UNORM_PACK16" ) == 0 ) {
        return VK_FORMAT_R12X4_UNORM_PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_R12X4G12X4_UNORM_2PACK16" ) == 0 ) {
        return VK_FORMAT_R12X4G12X4_UNORM_2PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16" ) == 0 ) {
        return VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16" ) == 0 ) {
        return VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16" ) == 0 ) {
        return VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16" ) == 0 ) {
        return VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16" ) == 0 ) {
        return VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16" ) == 0 ) {
        return VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16" ) == 0 ) {
        return VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16" ) == 0 ) {
        return VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16;
    }
    if ( strcmp( format, "VK_FORMAT_G16B16G16R16_422_UNORM" ) == 0 ) {
        return VK_FORMAT_G16B16G16R16_422_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_B16G16R16G16_422_UNORM" ) == 0 ) {
        return VK_FORMAT_B16G16R16G16_422_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM" ) == 0 ) {
        return VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM" ) == 0 ) {
        return VK_FORMAT_G16_B16R16_2PLANE_420_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM" ) == 0 ) {
        return VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM" ) == 0 ) {
        return VK_FORMAT_G16_B16R16_2PLANE_422_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM" ) == 0 ) {
        return VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM;
    }
    if ( strcmp( format, "VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG" ) == 0 ) {
        return VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG;
    }
    if ( strcmp( format, "VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG" ) == 0 ) {
        return VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG;
    }
    if ( strcmp( format, "VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG" ) == 0 ) {
        return VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG;
    }
    if ( strcmp( format, "VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG" ) == 0 ) {
        return VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG;
    }
    if ( strcmp( format, "VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG" ) == 0 ) {
        return VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG;
    }
    if ( strcmp( format, "VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG" ) == 0 ) {
        return VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG;
    }
    if ( strcmp( format, "VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG" ) == 0 ) {
        return VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG;
    }
    if ( strcmp( format, "VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG" ) == 0 ) {
        return VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT" ) == 0 ) {
        return VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT" ) == 0 ) {
        return VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT" ) == 0 ) {
        return VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT" ) == 0 ) {
        return VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT" ) == 0 ) {
        return VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT" ) == 0 ) {
        return VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT" ) == 0 ) {
        return VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT" ) == 0 ) {
        return VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT" ) == 0 ) {
        return VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT" ) == 0 ) {
        return VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT" ) == 0 ) {
        return VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT" ) == 0 ) {
        return VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT" ) == 0 ) {
        return VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT" ) == 0 ) {
        return VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT" ) == 0 ) {
        return VK_FORMAT_G8_B8R8_2PLANE_444_UNORM_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT" ) == 0 ) {
        return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT" ) == 0 ) {
        return VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT" ) == 0 ) {
        return VK_FORMAT_G16_B16R16_2PLANE_444_UNORM_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT" ) == 0 ) {
        return VK_FORMAT_A4R4G4B4_UNORM_PACK16_EXT;
    }
    if ( strcmp( format, "VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT" ) == 0 ) {
        return VK_FORMAT_A4B4G4R4_UNORM_PACK16_EXT;
    }
    return VK_FORMAT_UNDEFINED;
}