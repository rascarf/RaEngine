#include "VulkanFrameBuffer.h"
#include "VulkanContext.h"

void VulkanFrameBuffer::Init(VulkanContext* Context)
{
    {
        g_VulkanInstance = Context->GetVulkanInstance();
        
        CreateDepthStencil();
        CreateRenderPass();
        CreateFrameBuffers();
    }
}
