#pragma once
#include "Core/Core.h"
#include "VulkanCommonDefine.h"
#include "VulkanPipelineInfo.h"
#include "VulkanRenderTarget.h"
#include "VulkanBuffers/VulkanDynamicBufferRing.h"
#include "VulkanBuffers/VulkanTexture.h"
#include "VulkanShader/VulkanShader.h"

namespace ReEngine
{

    struct VulkanSimulateTexture
    {
        uint32 Set = 0;
        uint32 Binding = 0;
        VkDescriptorType  DescriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        VkShaderStageFlags StageFlags = 0;
        uint32 Count = 0;
        Ref<VulkanTexture> Texture = nullptr;
        std::vector<VkImageView> Textures;
    };

    struct VulkanSimulateBuffer
    {
        uint32 DataSize = 0;
        uint32 Set = 0;
        uint32 Binding = 0;
        uint32 DynamicIndex = 0;
        uint32 Count = 0;
        VkDescriptorType DescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        VkShaderStageFlags StageFlags = 0;
        VkDescriptorBufferInfo* BufferInfo;
    };

    class PipelineInterface
    {
        // 用于记录DynamicOffset以及速查当前运行的Shader是否有这些参数
        // 方便对于Set进行BufferView的管理,当作运行时的Descriptorset

        typedef std::unordered_map<std::string,VulkanSimulateBuffer> BuffersMap;
        typedef std::unordered_map<std::string,VulkanSimulateTexture> TexturesMap;
    public:
        virtual void Prepare(std::string MeshName = "default");
        virtual void PreparePipeline(){}

        virtual void SetUniform(const std::string& name, void* dataPtr, uint32 size);
        virtual void SetTexture(const std::string& name,Ref<VulkanTexture> texture,std::string MeshName= "default");
        virtual void SetTextures(const std::string& name,std::vector<VkImageView> ImageViews,std::string MeshName = "default");
        virtual void SetInputAttachment(const std::string& name, Ref<VulkanTexture> texture,std::string MeshName= "default");
        virtual void SetStorageBuffer(const std::string& name, Ref<VulkanBuffer> buffer,std::string MeshName= "default");
        virtual void SetStorageTexture(const std::string& name,Ref<VulkanTexture> texture,std::string MeshName = "default");
        virtual void PushConstant(Ref<VulkanCommandBuffer> cmd,uint32 Index,uint32 size,void* Data);
        
        virtual void BindDescriptorSets(VkCommandBuffer commandBuffer,VkPipelineBindPoint bindPoint,std::string MeshName = "default");

        virtual ~PipelineInterface();
    public:
        FORCE_INLINE VkPipeline GetVkPipeline()const
        {
            return mPipeline->Pipeline;
        }

        FORCE_INLINE VkPipelineLayout GetPipelineLayout()const
        {
            return mPipeline->PipelineLayout;
        }
    public:

        Ref<VulkanDynamicBufferRing> RingBuffer = nullptr;
        Ref<VulkanDevice> mVulkanDevice = nullptr;
        Ref<VulkanShader> mShader = nullptr;
        Ref<VulkanPipeline> mPipeline = nullptr;
        VulkanPipelineInfo mPipelineInfo;

        VkPipelineCache mPipelineCache;

        uint32 DynamicOffsetCount = 0;
        std::vector<uint32>     DynamicOffsets;

        BuffersMap              uniformBuffers;
        BuffersMap              storageBuffers;
        TexturesMap             textures;

        std::unordered_map<std::string,Ref<VulkanDescriptorSet>> DescriptorSets;
    };
    
    
    class VulkanGraphicPipeline : public PipelineInterface
    {
    public:

        static Ref<VulkanGraphicPipeline> Create(Ref<VulkanDevice> vulkanDevice,VkRenderPass RenderPass,VkPipelineCache PipelineCache,Ref<VulkanShader> Shader,Ref<VulkanDynamicBufferRing> RingBuffer);
        static Ref<VulkanGraphicPipeline> Create(Ref<VulkanDevice> vulkanDevice,Ref<VulkanRenderTarget>,VkPipelineCache PipelineCache,Ref<VulkanShader> Shader,Ref<VulkanDynamicBufferRing> RingBuffer);

        virtual void PreparePipeline();

    public:
        VkRenderPass mRenderPass;
    };

    class VulkanComputePipeline :public PipelineInterface
    {
    public:
        static Ref<VulkanComputePipeline> Create(Ref<VulkanDevice> vulkanDevice,VkPipelineCache PipelineCache,Ref<VulkanShader> Shader,Ref<VulkanDynamicBufferRing> RingBuffer);
        static Ref<VulkanComputePipeline> Create(Ref<VulkanDevice> vulkanDevice,VkPipelineCache PipelineCache,Ref<VulkanShader> Shader);

        void BindDispatch(VkCommandBuffer CommandBuffer,int groupX,int groupY,int groupZ);
        
        virtual void PreparePipeline() override;
    };
}

