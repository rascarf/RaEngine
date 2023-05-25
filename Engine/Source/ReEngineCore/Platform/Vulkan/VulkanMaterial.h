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
        std::vector<uint8> DataContent;
        bool Global = false;
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

    private:
         Ref<VulkanDynamicBufferRing> RingBuffer;

    public:
        virtual ~VulkanMaterial();

        static Ref<VulkanMaterial> Create(Ref<VulkanDevice> vulkanDevice,VkRenderPass RenderPass,VkPipelineCache PipelineCache,Ref<VulkanShader> Shader,Ref<VulkanDynamicBufferRing> RingBuffer);
        static Ref<VulkanMaterial> Create(Ref<VulkanDevice> vulkanDevice,Ref<VulkanRenderTarget>,VkPipelineCache PipelineCache,Ref<VulkanShader> Shader,Ref<VulkanDynamicBufferRing> RingBuffer);

        void PreparePipeline();
        void Prepare();

        void BeginObject();
        void EndObject();

        void BeginFrame();
        void EndFrame();

        void BindDescriptorSets(VkCommandBuffer commandBuffer,VkPipelineBindPoint bindPoint,int32 objIndex);
        void SetTexture(const std::string& name,Ref<VulkanTexture> texture);
        void SetLocalUniform(const std::string& name, void* dataPtr, uint32 size);
        void SetGlobalUniform(const std::string& name, void* dataPtr, uint32 size);
        void SetStorageBuffer(const std::string& name, Ref<VulkanBuffer> buffer);
        void SetInputAttachment(const std::string& name, Ref<VulkanTexture> texture);
    
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
        
        std::vector<uint32>     GlobalOffsets;
        std::vector<uint32>     DynamicOffsets;
        std::vector<uint32>     PerObjectIndexes;

        BuffersMap              uniformBuffers;
        BuffersMap              storageBuffers;
        TexturesMap             textures;

        bool                    actived = false;
    };
}

