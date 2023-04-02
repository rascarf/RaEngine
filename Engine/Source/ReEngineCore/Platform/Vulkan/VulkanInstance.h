#pragma once
#include "VulkanCommandPool.h"
#include "Core/Core.h"
#include "VulkanCommonDefine.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "Core/Window/GLFWWindow.h"

class VulkanInstance
{
public:
    VulkanInstance(GLFWwindow* m_WindowHandle,	const WindowProperty*	m_WindowInfo);
    virtual ~VulkanInstance();

    void Init();
    void PostInit();
    void Shutdown();

    FORCE_INLINE const std::vector<const char*>& GetInstanceExtensions() const
	{
		return m_InstanceExtensions;
	}

	FORCE_INLINE const std::vector<const char*>& GetInstanceLayers() const
	{
		return m_InstanceLayers;
	}

	FORCE_INLINE const VkInstance& GetInstance() const
	{
		return m_Instance;
	}

	FORCE_INLINE std::shared_ptr<VulkanDevice> GetDevice() const
	{
		return m_Device;
	}

	FORCE_INLINE std::shared_ptr<VulkanSwapChain> GetSwapChain() const
	{
		return m_SwapChain;
	}

	FORCE_INLINE const std::vector<VkImage>& GetBackbufferImages() const
	{
		return m_BackbufferImages;
	}

	FORCE_INLINE const std::vector<VkImageView>& GetBackbufferViews() const
	{
		return m_BackbufferViews;
	}

	FORCE_INLINE const PixelFormat& GetPixelFormat() const
	{
		return m_PixelFormat;
	}

	FORCE_INLINE const char* GetName()
	{ 
		return "Vulkan";
	}

	void AddAppDeviceExtensions(const char* name)
	{
		m_AppDeviceExtensions.push_back(name);
	}

	void AddAppInstanceExtensions(const char* name)
	{
		m_AppInstanceExtensions.push_back(name);
	}

	FORCE_INLINE void SetPhysicalDeviceFeatures(VkPhysicalDeviceFeatures2* deviceFeatures)
	{
		m_PhysicalDeviceFeatures2 = deviceFeatures;
	}

	FORCE_INLINE int32 GetWidth()
    {
	    return m_WindowInfo->Width;
    }

	FORCE_INLINE int32 GetHeight()
    {
    	return m_WindowInfo->Height;
    }

protected:

	void CreateInstance();

	void SelectAndInitDevice();

	static void GetInstanceLayersAndExtensions(std::vector<const char*>& outInstanceExtensions, std::vector<const char*>& outInstanceLayers);

	void SetupDebugCallBack();

	void InitInstance();

	void RecreateSwapChain();

	void DestorySwapChain();
	
	void DestroyDebugCallBack();
	
	void CreateSurface();

	GLFWwindow*							m_WindowHandle;
	const WindowProperty*				m_WindowInfo;
	
	VkInstance							m_Instance;
    std::vector<const char*>			m_InstanceLayers;
    std::vector<const char*>			m_InstanceExtensions;
	std::vector<const char*>			m_AppDeviceExtensions;
	std::vector<const char*>			m_AppInstanceExtensions;
	VkPhysicalDeviceFeatures2*			m_PhysicalDeviceFeatures2 = nullptr;

    std::shared_ptr<VulkanDevice>		m_Device;

    std::shared_ptr<VulkanSwapChain>	m_SwapChain;
	PixelFormat							m_PixelFormat;
	std::vector<VkImage>				m_BackbufferImages;
	std::vector<VkImageView>			m_BackbufferViews;
	
	VkDebugReportCallbackEXT			m_Callback;
	VkSurfaceKHR m_Surface;
	
};

FORCE_INLINE VkFormat PixelFormatToVkFormat(PixelFormat format, const bool bIsSRGB)
{
	VkFormat result = (VkFormat)G_PixelFormats[format].platformFormat;
	if (bIsSRGB)
	{
		switch (result)
		{
		case VK_FORMAT_B8G8R8A8_UNORM:				result = VK_FORMAT_B8G8R8A8_SRGB; break;
		case VK_FORMAT_A8B8G8R8_UNORM_PACK32:		result = VK_FORMAT_A8B8G8R8_SRGB_PACK32; break;
		case VK_FORMAT_R8_UNORM:					result = VK_FORMAT_R8_SRGB; break;
		case VK_FORMAT_R8G8_UNORM:					result = VK_FORMAT_R8G8_SRGB; break;
		case VK_FORMAT_R8G8B8_UNORM:				result = VK_FORMAT_R8G8B8_SRGB; break;
		case VK_FORMAT_R8G8B8A8_UNORM:				result = VK_FORMAT_R8G8B8A8_SRGB; break;
		case VK_FORMAT_BC1_RGB_UNORM_BLOCK:			result = VK_FORMAT_BC1_RGB_SRGB_BLOCK; break;
		case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:		result = VK_FORMAT_BC1_RGBA_SRGB_BLOCK; break;
		case VK_FORMAT_BC2_UNORM_BLOCK:				result = VK_FORMAT_BC2_SRGB_BLOCK; break;
		case VK_FORMAT_BC3_UNORM_BLOCK:				result = VK_FORMAT_BC3_SRGB_BLOCK; break;
		case VK_FORMAT_BC7_UNORM_BLOCK:				result = VK_FORMAT_BC7_SRGB_BLOCK; break;
		case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:		result = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK; break;
		case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:	result = VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK; break;
		case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:	result = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:		result = VK_FORMAT_ASTC_4x4_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:		result = VK_FORMAT_ASTC_5x4_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:		result = VK_FORMAT_ASTC_5x5_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:		result = VK_FORMAT_ASTC_6x5_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:		result = VK_FORMAT_ASTC_6x6_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:		result = VK_FORMAT_ASTC_8x5_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:		result = VK_FORMAT_ASTC_8x6_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:		result = VK_FORMAT_ASTC_8x8_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:		result = VK_FORMAT_ASTC_10x5_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:		result = VK_FORMAT_ASTC_10x6_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:		result = VK_FORMAT_ASTC_10x8_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:		result = VK_FORMAT_ASTC_10x10_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:		result = VK_FORMAT_ASTC_12x10_SRGB_BLOCK; break;
		case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:		result = VK_FORMAT_ASTC_12x12_SRGB_BLOCK; break;
		case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:	result = VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG; break;
		case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:	result = VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG; break;
		case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:	result = VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG; break;
		case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:	result = VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG; break;
		default:	break;
		}
	}
	return result;
}