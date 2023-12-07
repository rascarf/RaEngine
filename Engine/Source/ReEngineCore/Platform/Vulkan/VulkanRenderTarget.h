#pragma once

#include "VulkanCommonDefine.h"
#include "glm/vec4.hpp"
#include "VulkanBuffers/VulkanTexture.h"
namespace ReEngine
{
    struct VulkanRenderPassInfo
    {
        struct ColorEntry
        {
            Ref<VulkanTexture> RenderTarget = nullptr;
            Ref<VulkanTexture> ResolveTarget  = nullptr;

            VkAttachmentLoadOp LoadAction = VK_ATTACHMENT_LOAD_OP_CLEAR;
            VkAttachmentStoreOp StoreAction = VK_ATTACHMENT_STORE_OP_STORE;
        };

        struct DepthStencilEntry
        {
            Ref<VulkanTexture> DepthStencilTarget = nullptr;
            Ref<VulkanTexture> ResolveTarget = nullptr;

            VkAttachmentLoadOp LoadAction = VK_ATTACHMENT_LOAD_OP_CLEAR;
            VkAttachmentStoreOp StoreAction = VK_ATTACHMENT_STORE_OP_STORE;
        };

        ~VulkanRenderPassInfo()
        {
            for(auto& Texture : ColorRenderTargets)
            {
                Texture.RenderTarget.reset();
                Texture.ResolveTarget.reset();
            }

            DepthStencilRenderTarget.ResolveTarget.reset();
            DepthStencilRenderTarget.DepthStencilTarget.reset();
        }
        
        ColorEntry ColorRenderTargets[(int32)ResLimit::MaxSimultaneousRenderTargets];
        DepthStencilEntry DepthStencilRenderTarget;
        int32 NumColorRenderTargets;
        bool MultiView = false;

        // Color, no depth
        explicit VulkanRenderPassInfo(Ref<VulkanTexture> colorRT, VkAttachmentLoadOp colorLoadAction, VkAttachmentStoreOp colorStoreAction, Ref<VulkanTexture> resolveRT)
        {
            NumColorRenderTargets = 1;

            ColorRenderTargets[0].RenderTarget  = colorRT;
            ColorRenderTargets[0].ResolveTarget = resolveRT;
            ColorRenderTargets[0].LoadAction    = colorLoadAction;
            ColorRenderTargets[0].StoreAction   = colorStoreAction;

            DepthStencilRenderTarget.DepthStencilTarget = nullptr;
            DepthStencilRenderTarget.ResolveTarget      = nullptr;
            DepthStencilRenderTarget.LoadAction         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            DepthStencilRenderTarget.StoreAction        = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            memset(&ColorRenderTargets[NumColorRenderTargets], 0, sizeof(ColorEntry) * ((int)ResLimit::MaxSimultaneousRenderTargets - NumColorRenderTargets));
        }

        // Color And Depth
        explicit VulkanRenderPassInfo(
            Ref<VulkanTexture>  colorRT, 
            VkAttachmentLoadOp colorLoadAction, 
            VkAttachmentStoreOp colorStoreAction,
            Ref<VulkanTexture>  depthRT,
            VkAttachmentLoadOp depthLoadAction, 
            VkAttachmentStoreOp depthStoreAction
        )
        {
            NumColorRenderTargets = 1;

            ColorRenderTargets[0].RenderTarget  = colorRT;
            ColorRenderTargets[0].ResolveTarget = nullptr;
            ColorRenderTargets[0].LoadAction    = colorLoadAction;
            ColorRenderTargets[0].StoreAction   = colorStoreAction;

            DepthStencilRenderTarget.DepthStencilTarget = depthRT;
            DepthStencilRenderTarget.ResolveTarget      = nullptr;
            DepthStencilRenderTarget.LoadAction         = depthLoadAction;
            DepthStencilRenderTarget.StoreAction        = depthStoreAction;

            memset(&ColorRenderTargets[NumColorRenderTargets], 0, sizeof(ColorEntry) * ((int)ResLimit::MaxSimultaneousRenderTargets - NumColorRenderTargets));
        }
        
        // MRTs, No Depth
        explicit VulkanRenderPassInfo(int32 numColorRTs, std::vector<Ref<VulkanTexture>> colorRT, VkAttachmentLoadOp colorLoadAction, VkAttachmentStoreOp colorStoreAction)
        {
            NumColorRenderTargets = numColorRTs;

            for (int32 i = 0; i < numColorRTs; ++i)
            {
                ColorRenderTargets[i].RenderTarget  = colorRT[i];
                ColorRenderTargets[i].ResolveTarget = nullptr;
                ColorRenderTargets[i].LoadAction    = colorLoadAction;
                ColorRenderTargets[i].StoreAction   = colorStoreAction;
            }

            DepthStencilRenderTarget.DepthStencilTarget = nullptr;
            DepthStencilRenderTarget.ResolveTarget      = nullptr;
            DepthStencilRenderTarget.LoadAction         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            DepthStencilRenderTarget.StoreAction        = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            if (numColorRTs < (int)ResLimit::MaxSimultaneousRenderTargets)
            {
                memset(&ColorRenderTargets[NumColorRenderTargets], 0, sizeof(ColorEntry) * ((int)ResLimit::MaxSimultaneousRenderTargets - NumColorRenderTargets));
            }
        }
        // MRTs, No Depth, And LoadOps
        explicit VulkanRenderPassInfo(int32 numColorRTs, std::vector<Ref<VulkanTexture>> colorRT, std::vector<VkAttachmentLoadOp> colorLoadAction, VkAttachmentStoreOp colorStoreAction)
        {
            NumColorRenderTargets = numColorRTs;

            for (int32 i = 0; i < numColorRTs; ++i)
            {
                ColorRenderTargets[i].RenderTarget  = colorRT[i];
                ColorRenderTargets[i].ResolveTarget = nullptr;
                ColorRenderTargets[i].LoadAction    = colorLoadAction[i];
                ColorRenderTargets[i].StoreAction   = colorStoreAction;
            }

            DepthStencilRenderTarget.DepthStencilTarget = nullptr;
            DepthStencilRenderTarget.ResolveTarget      = nullptr;
            DepthStencilRenderTarget.LoadAction         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            DepthStencilRenderTarget.StoreAction        = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            if (numColorRTs < (int)ResLimit::MaxSimultaneousRenderTargets)
            {
                memset(&ColorRenderTargets[NumColorRenderTargets], 0, sizeof(ColorEntry) * ((int)ResLimit::MaxSimultaneousRenderTargets - NumColorRenderTargets));
            }
        }

        // MRTs And Depth
        explicit VulkanRenderPassInfo(
            int32 numColorRTs, 
            std::vector<Ref<VulkanTexture>> colorRT, 
            VkAttachmentLoadOp colorLoadAction, 
            VkAttachmentStoreOp colorStoreAction,
            Ref<VulkanTexture> depthRT,
            VkAttachmentLoadOp depthLoadAction, 
            VkAttachmentStoreOp depthStoreAction
        )
        {
            NumColorRenderTargets = numColorRTs;

            for (int32 i = 0; i < numColorRTs; ++i)
            {
                ColorRenderTargets[i].RenderTarget  = colorRT[i];
                ColorRenderTargets[i].ResolveTarget = nullptr;
                ColorRenderTargets[i].LoadAction    = colorLoadAction;
                ColorRenderTargets[i].StoreAction   = colorStoreAction;
            }
            
            DepthStencilRenderTarget.DepthStencilTarget = depthRT;
            DepthStencilRenderTarget.ResolveTarget    = nullptr;
            DepthStencilRenderTarget.LoadAction          = depthLoadAction;
            DepthStencilRenderTarget.StoreAction       = depthStoreAction;

            if (numColorRTs < (int)ResLimit::MaxSimultaneousRenderTargets)
            {
                memset(&ColorRenderTargets[NumColorRenderTargets], 0, sizeof(ColorEntry) * ((int)ResLimit::MaxSimultaneousRenderTargets - NumColorRenderTargets));
            }
        }

        // MRTs And Depth And LoadActions
        explicit VulkanRenderPassInfo(
            int32 numColorRTs, 
            std::vector<Ref<VulkanTexture>> colorRT, 
            std::vector<VkAttachmentLoadOp> colorLoadAction, 
            VkAttachmentStoreOp colorStoreAction,
            Ref<VulkanTexture> depthRT,
            VkAttachmentLoadOp depthLoadAction, 
            VkAttachmentStoreOp depthStoreAction
        )
        {
            NumColorRenderTargets = numColorRTs;

            for (int32 i = 0; i < numColorRTs; ++i)
            {
                ColorRenderTargets[i].RenderTarget  = colorRT[i];
                ColorRenderTargets[i].ResolveTarget = nullptr;
                ColorRenderTargets[i].LoadAction    = colorLoadAction[i];
                ColorRenderTargets[i].StoreAction   = colorStoreAction;
            }
            
            DepthStencilRenderTarget.DepthStencilTarget = depthRT;
            DepthStencilRenderTarget.ResolveTarget    = nullptr;
            DepthStencilRenderTarget.LoadAction          = depthLoadAction;
            DepthStencilRenderTarget.StoreAction       = depthStoreAction;

            if (numColorRTs < (int)ResLimit::MaxSimultaneousRenderTargets)
            {
                memset(&ColorRenderTargets[NumColorRenderTargets], 0, sizeof(ColorEntry) * ((int)ResLimit::MaxSimultaneousRenderTargets - NumColorRenderTargets));
            }
        }

        // Depth, No Color
        VulkanRenderPassInfo(Ref<VulkanTexture> depthRT, VkAttachmentLoadOp depthLoadAction, VkAttachmentStoreOp depthStoreAction)
        {
            NumColorRenderTargets = 0;

            DepthStencilRenderTarget.DepthStencilTarget = depthRT;
            DepthStencilRenderTarget.ResolveTarget    = nullptr;
            DepthStencilRenderTarget.LoadAction          = depthLoadAction;
            DepthStencilRenderTarget.StoreAction       = depthStoreAction;

            memset(&ColorRenderTargets[NumColorRenderTargets], 0, sizeof(ColorEntry) * (int)ResLimit::MaxSimultaneousRenderTargets);
        }
    };

    class VulkanRenderTargetLayout
    {
    public:
        VulkanRenderTargetLayout(const VulkanRenderPassInfo& RenderPassInfo);
        
        uint16 SetupSubpasses(
            VkSubpassDescription* OutDescs,
            uint32 MaxDescs,
            VkSubpassDependency* OutDeps,
            uint32 MaxDeps,
            uint32& OutNumDependencies
        )const;
        
    public:
        
        VkAttachmentReference   ColorReferences[(int)ResLimit::MaxSimultaneousRenderTargets];
        VkAttachmentReference   DepthStencilReference;
        VkAttachmentReference   ResolveReferences[(int)ResLimit::MaxSimultaneousRenderTargets];
        VkAttachmentReference   InputAttachments[(int)ResLimit::MaxSimultaneousRenderTargets + 1];
        
        VkAttachmentDescription Descriptions[(int)ResLimit::MaxSimultaneousRenderTargets * 2 + 1];

        uint8                   NumAttachmentDescriptions = 0;
        uint8                   NumColorAttachments = 0;
        uint8                   NumInputAttachments = 0;
        bool                    HasDepthStencil = false;
        bool                    HasResolveAttachments = false;
        VkSampleCountFlagBits   NumSamples = VK_SAMPLE_COUNT_1_BIT;
        uint8                   NumUsedClearValues = 0;

        VkExtent3D	            extent3D;
        bool					multiview = false;
    };

    class VulkanRenderPass
    {
    public:
        VulkanRenderPass(VkDevice InDevice,const VulkanRenderTargetLayout& RtLayout);
        virtual ~VulkanRenderPass()
        {
            if(RenderPass != VK_NULL_HANDLE)
            {
                vkDestroyRenderPass(Device,RenderPass,VULKAN_CPU_ALLOCATOR);
                RenderPass = VK_NULL_HANDLE;
            }
        }

    public:
        VulkanRenderTargetLayout Layout;
        uint32 NumUsedClearValues = 0;
        VkDevice Device = VK_NULL_HANDLE;
        VkRenderPass RenderPass = VK_NULL_HANDLE;
    };

    class VulkanFrameBuffer
    {
    public:
        VulkanFrameBuffer(VkDevice Device,const VulkanRenderTargetLayout& ,const VulkanRenderPass& renderPass,const VulkanRenderPassInfo& RenderPassInfo);
        ~VulkanFrameBuffer()
        {
            if(FrameBuffer != VK_NULL_HANDLE)
            {
                vkDestroyFramebuffer(Device,FrameBuffer,VULKAN_CPU_ALLOCATOR);
                FrameBuffer = VK_NULL_HANDLE;
            }
        }
        
    public:
        VkDevice Device = VK_NULL_HANDLE;

        VkFramebuffer FrameBuffer;
        uint32 NumColorRenderTargets = 0;
        uint32 NumColorAttachments = 0;

        std::vector<VkImageView> AttachmentTextureViews;
        VkImage ColorRenderTargetImages[(int)ResLimit::MaxSimultaneousRenderTargets];
        VkImage DepthStencilRenderTargetImage = VK_NULL_HANDLE;

        VkExtent2D Extent2D;
    };

    class VulkanRenderTarget
    {
    public:
        VulkanRenderTarget(const VulkanRenderPassInfo& InRenderPassInfo):RtLayout(InRenderPassInfo),RenderPassInfo(InRenderPassInfo),ClearColor(glm::vec4(0.f,0.f,0.f,1.f))
        {
            for (int32 i = 0; i < InRenderPassInfo.NumColorRenderTargets; ++i)
            {
                VkClearValue clearValue = {};
                clearValue.color={{ ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w }};
                ClearValues.push_back(clearValue);
            }

            if (InRenderPassInfo.DepthStencilRenderTarget.DepthStencilTarget)
            {
                VkClearValue clearValue = {};
                clearValue.depthStencil = { 1.0f, 0 };
                ClearValues.push_back(clearValue);
            }

            colorLayout = ImageLayoutBarrier::PixelShaderRead;
            depthLayout = ImageLayoutBarrier::PixelShaderRead;
        }

        VulkanRenderTarget(const VulkanRenderPassInfo& InRenderPassInfo, glm::vec4 inClearColor):RtLayout(InRenderPassInfo),RenderPassInfo(InRenderPassInfo),ClearColor(inClearColor)
        {
            for (int32 i = 0; i < InRenderPassInfo.NumColorRenderTargets; ++i)
            {
                VkClearValue clearValue = {};
                clearValue.color={{ ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w }};
                ClearValues.push_back(clearValue);
            }

            if (InRenderPassInfo.DepthStencilRenderTarget.DepthStencilTarget)
            {
                VkClearValue clearValue = {};
                clearValue.depthStencil = { 1.0f, 0 };
                ClearValues.push_back(clearValue);
            }

            colorLayout = ImageLayoutBarrier::PixelShaderRead;
            depthLayout = ImageLayoutBarrier::PixelShaderRead;
        }
    public:
        virtual ~VulkanRenderTarget()
        {
            mRenderPass.reset();
            mFrameBuffer.reset();
        }

        void BeginRenderPass(VkCommandBuffer CmdBuffer);
        void EndRenderPass(VkCommandBuffer CmdBuffer);

        FORCEINLINE VkRenderPass GetRenderPass()const
        {
            return mRenderPass->RenderPass;
        }

        FORCEINLINE VkFramebuffer GetFrameBuffer() const
        {
            return mFrameBuffer->FrameBuffer;
        }

        static Ref<VulkanRenderTarget> Create(Ref<VulkanDevice> VulkanDevice,const VulkanRenderPassInfo& RenderPassInfo);
        static Ref<VulkanRenderTarget> Create(Ref<VulkanDevice> VulkanDevice,const VulkanRenderPassInfo& RenderPassInfo,glm::vec4 ClearColor);

    public:
        VulkanRenderTargetLayout RtLayout;
        VulkanRenderPassInfo RenderPassInfo;
        glm::vec4 ClearColor;

        Ref<VulkanRenderPass> mRenderPass;
        Ref<VulkanFrameBuffer> mFrameBuffer;

        VkDevice InDevice;
        VkExtent2D Extent2D;
        std::vector<VkClearValue> ClearValues;

        ImageLayoutBarrier          colorLayout;
        ImageLayoutBarrier          depthLayout;
    };
}