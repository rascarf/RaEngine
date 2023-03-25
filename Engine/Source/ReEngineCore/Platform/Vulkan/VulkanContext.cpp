#include "VulkanContext.h"
#ifdef ANDROID
#include "vulkan_wrapper.h"
#else
#include "vulkan/vulkan.h"
#endif

namespace ReEngine
{
    VulkanContext::VulkanContext(GLFWwindow* windowHandle)
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    }

    void VulkanContext::Init()
    {
        
    }

    void VulkanContext::SwapBuffers()
    {
        
    }
}

