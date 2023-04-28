#pragma once
#include "VulkanTexture.h"
#include "Platform/Vulkan/VulkanCommonDefine.h"
#include "Platform/Vulkan/VulkanInstance.h"

namespace ReEngine
{
    class VulkanBackBuffer
{
public:
    VulkanBackBuffer(int32 width, int32 height, const char* title)
          : m_Width(width)
        , m_Height(height)
        , m_Title(title)
        , DepthStencil(nullptr)
        , m_RenderPass(VK_NULL_HANDLE)
        , m_SampleCount(VK_SAMPLE_COUNT_1_BIT)
        , m_DepthFormat(PF_DepthStencil)
    {

    }

    virtual ~VulkanBackBuffer()
    {

    }

    FORCE_INLINE void SetSize(int32 width, int32 height)
    {
        m_Width  = width;
        m_Height = height;
    }

    FORCE_INLINE int32 GetWidth() const
    {
        return m_Width;
    }

    FORCE_INLINE int32 GetHeight() const
    {
        return m_Height;
    }

    FORCE_INLINE const std::string& GetTitle()
    {
        return m_Title;
    }
    
    void Prepare()
    {

    }

    void ShutDown()
    {
        DestroyFrameBuffers();
        DestoryRenderPass();
        DestoryDepthStencil();
    }
    
    void Init(class VulkanContext* Context);


protected:
        
    virtual void CreateFrameBuffers()
    {
        DestroyFrameBuffers();
        
        const std::vector<VkImageView>& backbufferViews = g_VulkanInstance->GetBackbufferViews();
        int32 fwidth    = g_VulkanInstance->GetSwapChain()->GetWidth();
        int32 fheight   = g_VulkanInstance->GetSwapChain()->GetHeight();
        VkDevice device = g_VulkanInstance->GetDevice()->GetInstanceHandle();

        VkImageView attachments[2];
        attachments[1] = DepthStencil->ImageView;

        VkFramebufferCreateInfo frameBufferCreateInfo;
        ZeroVulkanStruct(frameBufferCreateInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
        frameBufferCreateInfo.renderPass      = m_RenderPass;
        frameBufferCreateInfo.attachmentCount = 2;
        frameBufferCreateInfo.pAttachments    = attachments;
        frameBufferCreateInfo.width           = fwidth;
        frameBufferCreateInfo.height          = fheight;
        frameBufferCreateInfo.layers          = 1;


        m_FrameBuffers.resize(backbufferViews.size());
        for (uint32 i = 0; i < m_FrameBuffers.size(); ++i)
        {
            attachments[0] = backbufferViews[i];
            VERIFYVULKANRESULT(vkCreateFramebuffer(device, &frameBufferCreateInfo, VULKAN_CPU_ALLOCATOR, &m_FrameBuffers[i]));
        }
        
    }

    virtual void CreateDepthStencil()
    {
        DepthStencil = VulkanTexture::CreateDepthStencil(m_Width,m_Height,g_VulkanInstance->GetDevice(),PixelFormat::PF_DepthStencil);
    }

    virtual void CreateRenderPass()
    {
        DestoryRenderPass();

        VkDevice device = g_VulkanInstance->GetDevice()->GetInstanceHandle();
        PixelFormat pixelFormat = g_VulkanInstance->GetPixelFormat();

        std::vector<VkAttachmentDescription> attachments(2);
        // color attachment
        attachments[0].format         = PixelFormatToVkFormat(pixelFormat, false);
        attachments[0].samples        = m_SampleCount;
        attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout    = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        
        // depth stencil attachment
        attachments[1].format         = PixelFormatToVkFormat(m_DepthFormat, false);
        attachments[1].samples        = m_SampleCount;
        attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorReference = { };
        colorReference.attachment = 0;
        colorReference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthReference = { };
        depthReference.attachment = 1;
        depthReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescription = { };
        subpassDescription.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount    = 1;
        subpassDescription.pColorAttachments       = &colorReference;
        subpassDescription.pDepthStencilAttachment = &depthReference;
        subpassDescription.pResolveAttachments     = nullptr;
        subpassDescription.inputAttachmentCount    = 0;
        subpassDescription.pInputAttachments       = nullptr;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments    = nullptr;
        
        VkRenderPassCreateInfo renderPassInfo;
        ZeroVulkanStruct(renderPassInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments    = attachments.data();
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpassDescription;
        VERIFYVULKANRESULT(vkCreateRenderPass(device, &renderPassInfo, VULKAN_CPU_ALLOCATOR, &m_RenderPass));
    }

    virtual void DestroyFrameBuffers()
    {
        VkDevice device = g_VulkanInstance->GetDevice()->GetInstanceHandle();
        
        for (int32 i = 0; i < m_FrameBuffers.size(); ++i)
        {
            vkDestroyFramebuffer(device, m_FrameBuffers[i], VULKAN_CPU_ALLOCATOR);
        }
        
        m_FrameBuffers.clear();
    }

    virtual void DestoryRenderPass()
    {
        VkDevice device = g_VulkanInstance->GetDevice()->GetInstanceHandle();
        if (m_RenderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(device, m_RenderPass, VULKAN_CPU_ALLOCATOR);
            m_RenderPass = VK_NULL_HANDLE;
        }
    }

    virtual void DestoryDepthStencil()
    {
        if(DepthStencil != nullptr)
        {
            DepthStencil.reset();
        }
    }

public:
    std::vector<const char*>    deviceExtensions;
    std::vector<const char*>    instanceExtensions;
    VkPhysicalDeviceFeatures2*  physicalDeviceFeatures = nullptr;

public:

    int32                       m_Width;
    int32                       m_Height;
    std::string                 m_Title;

    std::vector<VkFramebuffer>  m_FrameBuffers;
        
    VkRenderPass                m_RenderPass;
    VkSampleCountFlagBits       m_SampleCount;

    Ref<VulkanInstance>         g_VulkanInstance;
    Ref<VulkanTexture>          DepthStencil;
    PixelFormat                 m_DepthFormat;
};
}

