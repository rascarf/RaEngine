#include "VulkanFrameBuffer.h"
#include "Platform\Vulkan\VulkanContext.h"

void VulkanBackBuffer::Init(VulkanContext* Context)
{
    g_VulkanInstance = Context->GetVulkanInstance();

    CreateDepthStencil();
    CreateRenderPass();
    CreateFrameBuffers();
}
