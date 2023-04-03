#include "VulkanCommonDefine.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "Core/StringUtils.h"

#include <vector>

#include "VulkanInstance.h"

struct VulkanLayerExtension
{
	VulkanLayerExtension();
	void AddUniqueExtensionNames(std::vector<std::string>& outExtensions);
	void AddUniqueExtensionNames(std::vector<const char*>& outExtensions);

	VkLayerProperties layerProps;
	std::vector<VkExtensionProperties> extensionProps;
};

static const char* G_InstanceValidationLayers[] =
{
	"VK_LAYER_KHRONOS_validation",
	"VK_LAYER_RENDERDOC_Capture",
	nullptr
};

static const char* G_DeviceValidationLayers[] =
{
	"VK_LAYER_KHRONOS_validation",
	nullptr
};

static const char* G_InstanceExtensions[] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	nullptr
};

static const char* G_DeviceExtensions[] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_MAINTENANCE1_EXTENSION_NAME,
	nullptr
	// VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME,
	
};

static FORCE_INLINE void EnumerateInstanceExtensionProperties(const char* layerName, VulkanLayerExtension& outLayer)
{
	uint32 count = 0;
	vkEnumerateInstanceExtensionProperties(layerName, &count, nullptr);
	outLayer.extensionProps.resize(count);
	vkEnumerateInstanceExtensionProperties(layerName, &count, outLayer.extensionProps.data());
}

static FORCE_INLINE void EnumerateDeviceExtensionProperties(VkPhysicalDevice device, const char* layerName, VulkanLayerExtension& outLayer)
{
	uint32 count = 0;
	vkEnumerateDeviceExtensionProperties(device, layerName, &count, nullptr);
	outLayer.extensionProps.resize(count);
	vkEnumerateDeviceExtensionProperties(device, layerName, &count, outLayer.extensionProps.data());
}

static FORCE_INLINE int32 FindLayerIndexInList(const std::vector<VulkanLayerExtension>& layers, const char* layerName)
{
	for (int32 i = 0; i < layers.size(); ++i) 
	{
		if (strcmp(layers[i].layerProps.layerName, layerName) == 0) {
			return i;
		}
	}
	return -1;
}

static FORCE_INLINE bool FindLayerInList(const std::vector<VulkanLayerExtension>& layers, const char* layerName)
{
	return FindLayerIndexInList(layers, layerName) != -1;
}

static FORCE_INLINE bool FindLayerExtensionInList(const std::vector<VulkanLayerExtension>& layers, const char* extensionName, const char*& foundLayer)
{
	for (int32 i = 0; i < layers.size(); ++i) 
	{
		for (int32 j = 0; j < layers[i].extensionProps.size(); ++j)
		{
			if (strcmp(layers[i].extensionProps[j].extensionName, extensionName) == 0)
			{
				foundLayer = layers[i].layerProps.layerName;
				return true;
			}
		}
	}
	return false;
}

static FORCE_INLINE bool FindLayerExtensionInList(const std::vector<VulkanLayerExtension>& layers, const char* extensionName)
{
	const char* dummy = nullptr;
	return FindLayerExtensionInList(layers, extensionName, dummy);
}

static FORCE_INLINE void TrimDuplicates(std::vector<const char*>& arr)
{
	for (int32 i = (int32)arr.size() - 1; i >= 0; --i)
	{
		bool found = false;
		for (int32 j = i - 1; j >= 0; --j)
		{
			if (strcmp(arr[i], arr[j]) == 0)
			{
				found = true;
				break;
			}
		}
		
		if (found)
		{
			arr.erase(arr.begin() + i);
		}
	}
}

VulkanLayerExtension::VulkanLayerExtension()
{
	memset(&layerProps, 0, sizeof(VkLayerProperties));
}

void VulkanLayerExtension::AddUniqueExtensionNames(std::vector<std::string>& outExtensions)
{
	for (int32 i = 0; i < extensionProps.size(); ++i) {
		StringUtils::AddUnique(outExtensions, extensionProps[i].extensionName);
	}
}

void VulkanLayerExtension::AddUniqueExtensionNames(std::vector<const char*>& outExtensions)
{
	for (int32 i = 0; i < extensionProps.size(); ++i) {
		StringUtils::AddUnique(outExtensions, extensionProps[i].extensionName);
	}
}

void VulkanInstance::GetInstanceLayersAndExtensions(std::vector<const char*>& outInstanceExtensions, std::vector<const char*>& outInstanceLayers)
{
	//找到Instance扩展
	std::vector<VulkanLayerExtension> globalLayerExtensions(1);
	EnumerateInstanceExtensionProperties(nullptr, globalLayerExtensions[0]);

	std::vector<std::string> foundUniqueExtensions;
	for (int32 i = 0; i < globalLayerExtensions[0].extensionProps.size(); ++i)
	{
		StringUtils::AddUnique(foundUniqueExtensions, globalLayerExtensions[0].extensionProps[i].extensionName);
	}
	
	uint32 instanceLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
	std::vector<VkLayerProperties> globalLayerProperties(instanceLayerCount);
	vkEnumerateInstanceLayerProperties(&instanceLayerCount, globalLayerProperties.data());

	std::vector<std::string> foundUniqueLayers;
	for (int32 i = 0; i < globalLayerProperties.size(); ++i) 
	{
		VulkanLayerExtension layer;
		layer.layerProps = globalLayerProperties[i];
		EnumerateInstanceExtensionProperties(globalLayerProperties[i].layerName, layer);
		layer.AddUniqueExtensionNames(foundUniqueExtensions);
		StringUtils::AddUnique(foundUniqueLayers, globalLayerProperties[i].layerName);
		globalLayerExtensions.push_back(layer);
	}

	//取消注释来展示当前Instance的Extension
	// for (const std::string& name : foundUniqueLayers) 
	// {
	// 	RE_INFO("- Found instance layer {0}", name.c_str());
	// }
	//
	// for (const std::string& name : foundUniqueExtensions)
	// 	{
	// 	RE_INFO("- Found instance extension {0}", name.c_str());
	// }
	
#ifdef NDEBUG
	const bool EnableValidationLayers = false;
#else
	const bool EnableValidationLayers = true;
#endif
	
	if(EnableValidationLayers)
	{
		for (int32 i = 0; G_InstanceValidationLayers[i] != nullptr; ++i) 
		{
			const char* currValidationLayer = G_InstanceValidationLayers[i];
			bool found = FindLayerInList(globalLayerExtensions, currValidationLayer);
			if (found) {
				outInstanceLayers.push_back(currValidationLayer);
			} 
			else
			{
				RE_CORE_INFO("Unable to find Vulkan instance validation layer {0}", currValidationLayer);
			}
		}
	}

	if (FindLayerExtensionInList(globalLayerExtensions, VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
	{
		outInstanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}
	
	//TODO 这里之后改成按照平台读取
	//需要打开的Instance拓展
	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	
	std::vector<const char*> platformExtensions;
	platformExtensions.push_back(*glfwExtensions);
	
	for (const char* extension : platformExtensions)
	{
		if (FindLayerExtensionInList(globalLayerExtensions, extension))
		{
			outInstanceExtensions.push_back(extension);
		}
	}

	for (int32 i = 0; G_InstanceExtensions[i] != nullptr; ++i)
	{
		if (FindLayerExtensionInList(globalLayerExtensions, G_InstanceExtensions[i]))
		{
			outInstanceExtensions.push_back(G_InstanceExtensions[i]);
		}
	}
    
	TrimDuplicates(outInstanceLayers);
	if (outInstanceLayers.size() > 0) {
		RE_CORE_INFO("Using instance layers");
		for (const char* layer : outInstanceLayers)
		{
			RE_CORE_INFO("* {0}", layer);
		}
	}
	else
	{
		RE_CORE_INFO("Not using instance layers");
	}

	TrimDuplicates(outInstanceExtensions);
	if (outInstanceExtensions.size() > 0)
	{
		RE_CORE_INFO("Using instance extensions");
		for (const char* extension : outInstanceExtensions)
		{
			RE_CORE_INFO("* {0}", extension);
		}
	}
	else
	{
		RE_CORE_INFO("Not using instance extensions");
	}
}

void VulkanDevice::GetDeviceExtensionsAndLayers(std::vector<const char*>& outDeviceExtensions, std::vector<const char*>& outDeviceLayers, bool& bOutDebugMarkers)
{
	bOutDebugMarkers = false;
    
	uint32 count = 0;
	vkEnumerateDeviceLayerProperties(m_PhysicalDevice, &count, nullptr);
	std::vector<VkLayerProperties> properties(count);
	vkEnumerateDeviceLayerProperties(m_PhysicalDevice, &count, properties.data());
    
	std::vector<VulkanLayerExtension> deviceLayerExtensions(count + 1);
	for (int32 index = 1; index < deviceLayerExtensions.size(); ++index) {
		deviceLayerExtensions[index].layerProps = properties[index - 1];
	}
    
	std::vector<std::string> foundUniqueLayers;
	std::vector<std::string> foundUniqueExtensions;
	for (int32 index = 0; index < deviceLayerExtensions.size(); ++index)
	{
		if (index == 0) {
			EnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, deviceLayerExtensions[index]);
		}
		else {
			StringUtils::AddUnique(foundUniqueLayers, deviceLayerExtensions[index].layerProps.layerName);
			EnumerateDeviceExtensionProperties(m_PhysicalDevice, deviceLayerExtensions[index].layerProps.layerName, deviceLayerExtensions[index]);
		}
        
		deviceLayerExtensions[index].AddUniqueExtensionNames(foundUniqueExtensions);
	}

	//取消注释来展示当前设备的Extension
	// for (const std::string& name : foundUniqueLayers)
	// {
	// 	RE_CORE_INFO("- Found device layer {0}", name.c_str());
	// }
 //    
	// for (const std::string& name : foundUniqueExtensions)
	// {
	// 	RE_CORE_INFO("- Found device extension {0}", name.c_str());
	// }

#ifdef NDEBUG
	const bool EnableValidationLayers = false;
#else
	const bool EnableValidationLayers = true;
#endif
	
	if(EnableValidationLayers)
	{
		for (uint32 layerIndex = 0; G_DeviceValidationLayers[layerIndex] != nullptr; ++layerIndex)
		{
			bool bValidationFound = false;
			const char* currValidationLayer = G_DeviceValidationLayers[layerIndex];
			for (int32 index = 1; index < deviceLayerExtensions.size(); ++index)
			{
				if (strcmp(deviceLayerExtensions[index].layerProps.layerName, currValidationLayer) == 0)
				{
					bValidationFound = true;
					outDeviceLayers.push_back(currValidationLayer);
					break;
				}
			}
			    
			if (!bValidationFound) {
				RE_CORE_INFO("Unable to find Vulkan device validation layer '{0}'", currValidationLayer);
			}
		}
	}

	//所有可以加载的Extension
    std::vector<const char*> availableExtensions;
    for (int32 extIndex = 0; extIndex < deviceLayerExtensions[0].extensionProps.size(); ++extIndex)
    {
        availableExtensions.push_back(deviceLayerExtensions[0].extensionProps[extIndex].extensionName);
    }
    
    for (int32 layerIndex = 0; layerIndex < outDeviceLayers.size(); ++layerIndex)
    {
        int32 findLayerIndex;
        for (findLayerIndex = 1; findLayerIndex < deviceLayerExtensions.size(); ++findLayerIndex)
        {
            if (strcmp(deviceLayerExtensions[findLayerIndex].layerProps.layerName, outDeviceLayers[layerIndex]) == 0) {
                break;
            }
        }
        
        if (findLayerIndex < deviceLayerExtensions.size())
        {
            deviceLayerExtensions[findLayerIndex].AddUniqueExtensionNames(availableExtensions);
        }
    }
    
    TrimDuplicates(availableExtensions);
    
    auto ListContains = [](const std::vector<const char*>& arr, const char* name) -> bool
    {
        for (const char* element : arr)
        {
            if (strcmp(element, name) == 0) {
                return true;
            }
        }
        return false;
    };

	//TODO 这里之后改成按照平台读取
	//需要打开的设备Extension
    std::vector<const char*> platformExtensions;

	platformExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	platformExtensions.push_back("VK_KHR_shader_non_semantic_info");
	
    for (const char* platformExtension : platformExtensions)
    {
        if (ListContains(availableExtensions, platformExtension))
        {
            outDeviceExtensions.push_back(platformExtension);
        }
    }
    
    for (uint32 index = 0; G_DeviceExtensions[index] != nullptr; ++index)
    {
        if (ListContains(availableExtensions, G_DeviceExtensions[index])) {
            outDeviceExtensions.push_back(G_DeviceExtensions[index]);
        }
    }
    
    if (outDeviceExtensions.size() > 0)
    {
        RE_CORE_INFO("Using device extensions");
        for (const char* extension : outDeviceExtensions) {
            RE_CORE_INFO("* {0}", extension);
        }
    }
    
    if (outDeviceLayers.size() > 0)
    {
        RE_CORE_INFO("Using device layers");
        for (const char* layer : outDeviceLayers) {
            RE_CORE_INFO("* {0}", layer);
        }
    }
}
