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
        Ref<VulkanTexture> Texture = nullptr;
    };

    struct VulkanSimulateBuffer
    {
        uint32 DataSize = 0;
        uint32 Set = 0;
        uint32 Binding = 0;
        uint32 DynamicIndex = 0;
        VkDescriptorType DescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        VkShaderStageFlags StageFlags = 0;
        VkDescriptorBufferInfo* BufferInfo;
    };
    
    class VulkanMaterial
    {
    private:
        typedef std::unordered_map<std::string,VulkanSimulateBuffer> BuffersMap;
        typedef std::unordered_map<std::string,VulkanSimulateTexture> TexturesMap;

    public:
        virtual ~VulkanMaterial();

        static Ref<VulkanMaterial> Create(Ref<VulkanDevice> vulkanDevice,VkRenderPass RenderPass,VkPipelineCache PipelineCache,Ref<VulkanShader> Shader,Ref<VulkanDynamicBufferRing> RingBuffer);
        static Ref<VulkanMaterial> Create(Ref<VulkanDevice> vulkanDevice,Ref<VulkanRenderTarget>,VkPipelineCache PipelineCache,Ref<VulkanShader> Shader,Ref<VulkanDynamicBufferRing> RingBuffer);

        void PreparePipeline();
        void Prepare();

        void BindDescriptorSets(VkCommandBuffer commandBuffer,VkPipelineBindPoint bindPoint);
        
        void SetLocalUniform(const std::string& name, void* dataPtr, uint32 size);
        void SetLocalUniform(const std::string& name,VkDescriptorBufferInfo BufferView);
        
        void SetTexture(const std::string& name,Ref<VulkanTexture> texture);
        void SetInputAttachment(const std::string& name, Ref<VulkanTexture> texture);
    
        void SetStorageBuffer(const std::string& name, Ref<VulkanBuffer> buffer);
    public:
        VulkanMaterial();

    public:
        Ref<VulkanDevice> mVulkanDevice = nullptr;
        Ref<VulkanShader> mShader = nullptr;

        VkRenderPass mRenderPass;
        VkPipelineCache mPipelineCache;

        VulkanPipelineInfo mPipelineInfo;
        Ref<VulkanPipeline> mPipeline;
        Ref<VulkanDescriptorSet> DescriptorSet;

        uint32 DynamicOffsetCount;
        std::vector<uint32>     DynamicOffsets;

        BuffersMap              uniformBuffers;
        BuffersMap              storageBuffers;
        TexturesMap             textures;

        bool                    actived = false;

    private:
        Ref<VulkanDynamicBufferRing> RingBuffer;
    };

    class VulkanComputeMaterial
    {
    private:
        // 用于记录DynamicOffset以及速查当前运行的Shader是否有这些参数
        // 方便对于Set进行BufferView的管理
        typedef std::unordered_map<std::string,VulkanSimulateBuffer> BuffersMap;
        typedef std::unordered_map<std::string,VulkanSimulateTexture> TexturesMap;
    
    public:
        VulkanComputeMaterial(){}
        ~VulkanComputeMaterial();

    public:
        static Ref<VulkanComputeMaterial> Create(Ref<VulkanDevice> vulkanDevice,VkPipelineCache PipelineCache,Ref<VulkanShader> Shader,Ref<VulkanDynamicBufferRing> RingBuffer);

        void BindDispatch(VkCommandBuffer CommandBuffer,int groupX,int groupY,int groupZ);
        void BindDecriptorSets(VkCommandBuffer commandBuffer,VkPipelineBindPoint BindPoint);

        void SetUniform(const std::string& name, void* dataPtr, uint32 size);
        void SetUniform(const std::string& name,VkDescriptorBufferInfo BufferView);
        void SetTexture(const std::string& name,Ref<VulkanTexture> texture);

        void SetStorageBuffer(const std::string& name, Ref<VulkanBuffer> buffer);
        void SetStorageTexture(const std::string& name,Ref<VulkanTexture> texture);

    public:
        FORCE_INLINE VkPipeline GetVkPipeline()const
        {
            return ComputePipeline;
        }

        FORCE_INLINE VkPipelineLayout GetPipelineLayout()const
        {
            return mShader->pipelineLayout;
        }

        FORCE_INLINE std::vector<VkDescriptorSet>& GetDescriptorSets()const
        {
            return DescriptorSet->DescriptorSets;
        }
        
    private:
        void PreparePipeline();
        void Prepare();
    
    private:
        Ref<VulkanDevice> mVulkanDevice = nullptr;
        Ref<VulkanShader> mShader = nullptr;

        VkPipelineCache mPipelineCache = VK_NULL_HANDLE;
        VkPipeline ComputePipeline = VK_NULL_HANDLE;
        
        Ref<VulkanDescriptorSet> DescriptorSet;

        uint32 DynamicOffsetCount;
        std::vector<uint32>     DynamicOffsets;

        BuffersMap              uniformBuffers;
        BuffersMap              storageBuffers;
        TexturesMap             textures;
        
        Ref<VulkanDynamicBufferRing> RingBuffer;
    };
}

