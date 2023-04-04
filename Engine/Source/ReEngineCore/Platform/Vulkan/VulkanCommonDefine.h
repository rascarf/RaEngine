#pragma once
#include "Core/Core.h"
#include "Renderer/RHI/PixelFormat.h"

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/Include/vulkan/vulkan.h"

#define VULKAN_CPU_ALLOCATOR nullptr

using namespace ReEngine;

template<class T>
static  void ZeroVulkanStruct(T& vkStruct, VkStructureType vkType)
{
    vkStruct.sType = vkType;
    memset(((uint8*)&vkStruct) + sizeof(VkStructureType), 0, sizeof(T) - sizeof(VkStructureType));
}

#define VERIFYVULKANRESULT(VkFunction)				{ const VkResult scopedResult = VkFunction; if (scopedResult != VK_SUCCESS) { RE_CORE_INFO("VKResult=%d,Function=%s,File=%s,Line=%d", scopedResult, #VkFunction, __FILE__, __LINE__); }}
#define VERIFYVULKANRESULT_EXPANDED(VkFunction)		{ const VkResult scopedResult = VkFunction; if (scopedResult < VK_SUCCESS)  { RE_CORE_INFO("VKResult=%d,Function=%s,File=%s,Line=%d", scopedResult, #VkFunction, __FILE__, __LINE__); }}

template< class T >
static FORCE_INLINE T Clamp(const T x, const T inMin, const T inMax)
{
    return x < inMin ? inMin : x < inMax ? x : inMax;
}

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