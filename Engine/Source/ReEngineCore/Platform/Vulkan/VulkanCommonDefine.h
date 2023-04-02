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