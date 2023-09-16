#include "VulkanInstance.h"
#include "GLFW/glfw3.h"


VulkanInstance::VulkanInstance(GLFWwindow* InWindowHandle, const WindowProperty*	WindowInfo):
  m_WindowHandle(InWindowHandle)
, m_WindowInfo(WindowInfo)
, m_Instance(VK_NULL_HANDLE)
, m_Device(nullptr)
, m_SwapChain(nullptr)
, m_PixelFormat(PF_B8G8R8A8)
{
    
}

VulkanInstance::~VulkanInstance()
{
    
}

void VulkanInstance::Init()
{
    InitInstance();
    SetupDebugCallBack();
    SelectAndInitDevice();
    CreateSurface();
    RecreateSwapChain();
}

void VulkanInstance::PostInit()
{
}

void VulkanInstance::Shutdown()
{
    DestorySwapChain();

    vkDestroySurfaceKHR(m_Instance, m_Surface, VULKAN_CPU_ALLOCATOR);

#ifndef NDEBUG
    DestroyDebugCallBack();
#endif
    
    m_Device->Destroy();
    m_Device = nullptr;

    vkDestroyInstance(m_Instance, VULKAN_CPU_ALLOCATOR);
}

void VulkanInstance::CreateInstance()
{
}

void VulkanInstance::SelectAndInitDevice()
{
    uint32_t gpuCount = 0;
    VkResult result = vkEnumeratePhysicalDevices(m_Instance, &gpuCount, nullptr);

    if (result == VK_ERROR_INITIALIZATION_FAILED)
    {
        RE_CORE_INFO("%s\n", "Cannot find a compatible Vulkan device or driver. Try updating your video driver to a more recent version and make sure your video card supports Vulkan.");
        return;
    }
    
    if (gpuCount == 0)
    {
        RE_CORE_INFO("%s\n", "Couldn't enumerate physical devices! Make sure your drivers are up to date and that you are not pending a reboot.");
        return;
    }

    RE_CORE_INFO("Found {0} device(s)", gpuCount);

    std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
    vkEnumeratePhysicalDevices(m_Instance, &gpuCount, physicalDevices.data());
    
    struct DeviceInfo
    {
        std::shared_ptr<VulkanDevice> device;
        uint32 deviceIndex;
    };

    std::vector<DeviceInfo> discreteDevices;
    std::vector<DeviceInfo> integratedDevices;

    for (uint32 index = 0; index < gpuCount; ++index)
    {
        std::shared_ptr<VulkanDevice> newDevice = std::make_shared<VulkanDevice>(physicalDevices[index]);
        bool isDiscrete = newDevice->QueryGPU(index);
        if (isDiscrete)
        {
            discreteDevices.push_back({newDevice, index});
        }
        else
        {
            integratedDevices.push_back({newDevice, index});
        }
    }

    for (int32 index = 0; index < integratedDevices.size(); ++index)
    {
        discreteDevices.push_back(integratedDevices[index]);
    }

    int32 deviceIndex = -1;
    if (discreteDevices.size() > 0)
    {
        int32 preferredVendor = -1;
        if (discreteDevices.size() > 1 && preferredVendor != -1)
        {
            for (int32 index = 0; index < discreteDevices.size(); ++index)
            {
                if (discreteDevices[index].device->GetDeviceProperties().properties.vendorID == preferredVendor)
                {
                    m_Device = discreteDevices[index].device;
                    deviceIndex = discreteDevices[index].deviceIndex;
                    break;
                }
            }
        }
        
        if (deviceIndex == -1)
        {
            m_Device = discreteDevices[0].device;
            deviceIndex = discreteDevices[0].deviceIndex;
        }
    }
    else
    {
        RE_CORE_INFO("%s", "No devices found!");
        deviceIndex = -1;
        return;
    }
    
    for (int32 i = 0; i < m_AppDeviceExtensions.size(); ++i)
    {
        m_Device->AddAppDeviceExtensions(m_AppDeviceExtensions[i]);
    }

    m_Device->InitGPU(deviceIndex);
}

void VulkanInstance::InitInstance()
{
    // 使用volk，就不用很麻烦地自己Load函数了
    volkInitialize();
    
    //创建扩展
    GetInstanceLayersAndExtensions(m_InstanceExtensions, m_InstanceLayers);

    if (m_AppInstanceExtensions.size() > 0)
    {
        RE_CORE_INFO("Using app instance extensions");
        for (int32 i = 0; i < m_AppInstanceExtensions.size(); ++i)
        {
            m_InstanceExtensions.push_back(m_AppInstanceExtensions[i]);
            RE_CORE_INFO("* {0}", m_AppInstanceExtensions[i]);
        }
    }
    
    VkApplicationInfo AppInfo = {};
    AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pNext = nullptr;
    AppInfo.pApplicationName = "Vulkan Application";
    AppInfo.pEngineName = "No Engine";
    AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 1);
    AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 1);
    AppInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo instanceCreateInfo;
    ZeroVulkanStruct(instanceCreateInfo, VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
    instanceCreateInfo.pApplicationInfo        = &AppInfo;
    instanceCreateInfo.enabledExtensionCount   = uint32_t(m_InstanceExtensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = m_InstanceExtensions.size() > 0 ? m_InstanceExtensions.data() : nullptr;
    instanceCreateInfo.enabledLayerCount       = uint32_t(m_InstanceLayers.size());
    instanceCreateInfo.ppEnabledLayerNames     = m_InstanceLayers.size() > 0 ? m_InstanceLayers.data() : nullptr;

    VkResult result = vkCreateInstance(&instanceCreateInfo, VULKAN_CPU_ALLOCATOR, &m_Instance);
    if (result == VK_ERROR_INCOMPATIBLE_DRIVER) 
    {
        RE_CORE_INFO("{0}", "Cannot find a compatible Vulkan driver (ICD).");
    }
    else if (result == VK_ERROR_EXTENSION_NOT_PRESENT)
    {
        std::string missingExtensions;
        uint32 propertyCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);
        std::vector<VkExtensionProperties> properties(propertyCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, properties.data());

        for (const char* extension : m_InstanceExtensions)
        {
            bool found = false;
            for (uint32 i = 0; i < propertyCount; ++i)
            {
                const char* propExtension = properties[i].extensionName;
                if (strcmp(propExtension, extension) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                std::string extensionStr(extension);
                missingExtensions += extensionStr + "\n";
            }
        }

        RE_CORE_ERROR("Vulkan driver doesn't contain specified extensions:\n{0}", missingExtensions.c_str());
    }
    else if (result != VK_SUCCESS)
    {
        RE_CORE_ERROR("Vulkan failed to create instance.");
    }
    else {
        RE_CORE_ERROR("Vulkan successed to create instance.");
    }

    volkLoadInstance(m_Instance);
}

void VulkanInstance::RecreateSwapChain()
{
    DestorySwapChain();

    uint32 desiredNumBackBuffers = 3;
    int32 width  = m_WindowInfo->Width;
    int32 height = m_WindowInfo->Height;
    
    m_SwapChain  = std::shared_ptr<VulkanSwapChain>(new VulkanSwapChain(m_Instance, m_Device, m_Surface, m_PixelFormat, width, height, &desiredNumBackBuffers, m_BackbufferImages, 1));
	
    m_BackbufferViews.resize(m_BackbufferImages.size());
    for (int32 i = 0; i < m_BackbufferViews.size(); ++i)
    {
        VkImageViewCreateInfo imageViewCreateInfo;
        ZeroVulkanStruct(imageViewCreateInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
        imageViewCreateInfo.format     = PixelFormatToVkFormat(m_PixelFormat, false);
        imageViewCreateInfo.components = m_Device->GetFormatComponentMapping(m_PixelFormat);
        imageViewCreateInfo.viewType   = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.image      = m_BackbufferImages[i];
        imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
        imageViewCreateInfo.subresourceRange.levelCount     = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount     = 1;
        VERIFYVULKANRESULT(vkCreateImageView(m_Device->GetInstanceHandle(), &imageViewCreateInfo, VULKAN_CPU_ALLOCATOR, &(m_BackbufferViews[i])));
    }
}

void VulkanInstance::CreateSurface()
{
    {
        if (glfwCreateWindowSurface(m_Instance, m_WindowHandle, nullptr, &m_Surface) != VK_SUCCESS)
        {
            RE_CORE_ERROR("failed to create window surface!");
        }
    }
}

void VulkanInstance::DestorySwapChain()
{
    if(m_SwapChain)
    {
        m_SwapChain->ShutDown();
    }
    
    m_SwapChain = nullptr;

    for (int32 i = 0; i < m_BackbufferViews.size(); ++i)
    {
        vkDestroyImageView(m_Device->GetInstanceHandle(), m_BackbufferViews[i], VULKAN_CPU_ALLOCATOR);
    }
}

/* ================================================================Debug=================================================================================== */
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags,VkDebugReportObjectTypeEXT objType,uint64_t obj,size_t location,int32_t code,const char* layerPrefix,const char* msg,void* userData)
{
    std::string prefix("");
        
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        prefix += "ERROR:";
    }
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        prefix += "WARNING:";
    }
    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        prefix += "PERFORMANCE:";
    }
    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
        prefix += "INFO:";
    }
    if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
        prefix += "DEBUG:";
    }
        
    RE_CORE_INFO("{0} {1} {2} : {3}", prefix.c_str(), layerPrefix, code, msg);
        
    return VK_FALSE;
}

VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
    auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pCallback);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanInstance::SetupDebugCallBack()
{
    
#ifdef NDEBUG
    const bool EnableValidationLayers = false;
#else
    const bool EnableValidationLayers = true;
#endif
    
    if (!EnableValidationLayers) return;

    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = debugCallback;

    if (CreateDebugReportCallbackEXT(m_Instance, &createInfo, nullptr, &m_Callback) != VK_SUCCESS)
    {
        RE_CORE_ERROR("failed to set up debug callback!");
    }
}

void VulkanInstance::DestroyDebugCallBack()
{
    auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugReportCallbackEXT");
    if (func != nullptr) {
        func(m_Instance, m_Callback, VULKAN_CPU_ALLOCATOR);
    }
}


