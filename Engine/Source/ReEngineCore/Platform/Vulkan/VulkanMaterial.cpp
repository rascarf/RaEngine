#include "VulkanMaterial.h"

VulkanMaterial::~VulkanMaterial()
{
    mShader.reset();
    DescriptorSet.reset();

    textures.clear();
    uniformBuffers.clear();

    mVulkanDevice.reset();

    if(mPipeline)
    {
        mPipeline.reset();
    }

    RingBuffer.reset();
}

Ref<VulkanMaterial> VulkanMaterial::Create(Ref<VulkanDevice> vulkanDevice, VkRenderPass RenderPass,VkPipelineCache PipelineCache, Ref<VulkanShader> Shader,Ref<VulkanDynamicBufferRing> InRingBuffer)
{
    Ref<VulkanMaterial> Material = CreateRef<VulkanMaterial>();
    Material->mVulkanDevice = vulkanDevice;
    Material->mShader = Shader;
    Material->mRenderPass = RenderPass;
    Material->mPipelineCache = PipelineCache;
    Material->RingBuffer = InRingBuffer;

    Material->Prepare();

    return Material;
}

Ref<VulkanMaterial> VulkanMaterial::Create(Ref<VulkanDevice> vulkanDevice, Ref<VulkanRenderTarget> RT,VkPipelineCache PipelineCache, Ref<VulkanShader> Shader,Ref<VulkanDynamicBufferRing> InRingBuffer)
{
    // 创建材质
    Ref<VulkanMaterial> Material = CreateRef<VulkanMaterial>();
    Material->mPipelineInfo.ColorAttachmentsCount = RT->RenderPassInfo.NumColorRenderTargets;
    Material->mVulkanDevice  = vulkanDevice;
    Material->mShader        = Shader;
    Material->mRenderPass    = RT->GetRenderPass();
    Material->mPipelineCache = PipelineCache;
    Material->RingBuffer     = InRingBuffer;
    
    Material->Prepare();

    return Material;
}

void VulkanMaterial::PreparePipeline()
{
    if(mPipeline)
    {
        mPipeline.reset();
    }

    mPipelineInfo.Shader = mShader;
    mPipeline = VulkanPipeline::Create(
        mVulkanDevice,
        mPipelineCache,
        mPipelineInfo,
        mShader->inputBindings,
        mShader->inputAttributes,
        mShader->pipelineLayout,
        mRenderPass
        );
}

void VulkanMaterial::Prepare()
{
    DescriptorSet = mShader->AllocateDescriptorSet();
    for(auto it = mShader->bufferParams.begin(); it != mShader->bufferParams.end();++it)
    {
        VulkanSimulateBuffer UniformBuffer = {};
        UniformBuffer.Binding = it->second.Binding;
        UniformBuffer.DescriptorType = it->second.DescriptorType;
        UniformBuffer.Set = it->second.Set;
        UniformBuffer.StageFlags     = it->second.StageFlags;
        UniformBuffer.DataSize       = it->second.BufferSize;
        UniformBuffer.BufferInfo     = RingBuffer->GetSetDescriptor(UniformBuffer.DataSize);

        if(it->second.DescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||it->second.DescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
        {
            uniformBuffers.insert(std::make_pair(it->first,UniformBuffer));
            DescriptorSet->WriteBuffer(it->first,UniformBuffer.BufferInfo);
        }
        else if(it->second.DescriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||it->second.DescriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC)
        {
            storageBuffers.insert(std::make_pair(it->first, UniformBuffer));
        }
    }

    //有多少个需要设置的索引，每个set可能都不同
    //DynamicOffset的顺序和Set还有binding有关
    DynamicOffsetCount = 0;

    //遍历所有的Set
    std::vector<VulkanDescriptorSetLayoutInfo>& SetLayouts = mShader->SetLayoutsInfo.SetLayouts;
    for(int32 i = 0 ; i < SetLayouts.size() ; i++)
    {
        std::vector<VkDescriptorSetLayoutBinding>& Bindings = SetLayouts[i].Bindings;

        //遍历所有的Binding
        for(int32 BindingIndex = 0 ; BindingIndex < Bindings.size();BindingIndex++)
        {
            //对比是不是Dynamic的
            for(auto it = uniformBuffers.begin(); it != uniformBuffers.end(); it++)
            {
                if (it->second.Set == SetLayouts[i].Set && it->second.Binding == Bindings[BindingIndex].binding &&it->second.DescriptorType == Bindings[BindingIndex].descriptorType &&it->second.StageFlags == Bindings[BindingIndex].stageFlags)
                {
                    it->second.DynamicIndex = DynamicOffsetCount;
                    DynamicOffsetCount += 1;
                    break;
                }
            }
        }
    }

    DynamicOffsets.resize(DynamicOffsetCount);
    
    // 从Shader中获取Texture信息，包含attachment信息
    for (auto it = mShader->imageParams.begin(); it != mShader->imageParams.end(); ++it)
    {
        VulkanSimulateTexture texture = {};
        texture.Texture         = nullptr;
        texture.Binding         = it->second.Binding;
        texture.DescriptorType  = it->second.DescriptorType;
        texture.Set             = it->second.Set;
        texture.StageFlags      = it->second.StageFlags;
        textures.insert(std::make_pair(it->first, texture));
    }
}


void VulkanMaterial::BindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint)
{
    vkCmdBindDescriptorSets(
            commandBuffer,
            bindPoint,
            mPipeline->PipelineLayout,
            0,
            (uint32_t)DescriptorSet->DescriptorSets.size(),
            DescriptorSet->DescriptorSets.data(),
            DynamicOffsetCount,
            DynamicOffsets.data()
    );
}

void VulkanMaterial::SetTexture(const std::string& name, Ref<VulkanTexture> texture)
{
    auto it = textures.find(name);
    if(it == textures.end())
    {
        RE_CORE_ERROR("Texture {0} not found. ",name);
        return;
    }

    if(texture == nullptr)
    {
        RE_CORE_ERROR("Texture {0} can't be null. ",name);
    }

    if(it->second.Texture != texture)
    {
        it->second.Texture = texture;
        DescriptorSet->WriteImage(name,texture);
    }
}

void VulkanMaterial::SetInputAttachment(const std::string& name, Ref<VulkanTexture> texture)
{
    SetTexture(name,texture);
}

void VulkanMaterial::SetStorageBuffer(const std::string& name, Ref<VulkanBuffer> buffer)
{
    auto it = storageBuffers.find(name);
    if (it == storageBuffers.end())
    {
        RE_CORE_ERROR("StorageBuffer {0} not found.", name.c_str());
        return;
    }

    if (buffer == nullptr)
    {
        RE_CORE_ERROR("StorageBuffer {0} can't be null.", name.c_str());
        return;
    }

    if (it->second.BufferInfo->buffer != buffer->Buffer)
    {
        it->second.DataSize          = (uint32)buffer->Size;
        it->second.BufferInfo->buffer = buffer->Buffer;
        it->second.BufferInfo->offset = 0;
        it->second.BufferInfo->range  = buffer->Size;
        DescriptorSet->WriteBuffer(name, buffer);
    }
}

void VulkanMaterial::SetLocalUniform(const std::string& name, void* dataPtr, uint32 size)
{
    auto it = uniformBuffers.find(name);
    if(it == uniformBuffers.end())
    {
        RE_CORE_ERROR("Uniform {0} not found.", name.c_str());
    }

    // if (it->second.DataSize != size)
    // {
    //     RE_CORE_ERROR("Uniform {0} size not match, dst={1} src={2}", name.c_str(), it->second.DataSize, size);
    //     return;
    // }

    const auto BufferView =  RingBuffer->AllocConstantBuffer(it->second.DataSize,dataPtr);
    
    // 获取Object的起始位置以及DynamicOffset的起始位置
    uint32* dynOffsets = DynamicOffsets.data();

    // 记录Offset
    dynOffsets[it->second.DynamicIndex] = (uint32)BufferView.offset;
}

void VulkanMaterial::SetLocalUniform(const std::string& name, VkDescriptorBufferInfo BufferView)
{
    auto it = uniformBuffers.find(name);
    if(it == uniformBuffers.end())
    {
        RE_CORE_ERROR("Uniform {0} not found.", name.c_str());
    }

    // 获取Object的起始位置以及DynamicOffset的起始位置
    uint32* dynOffsets = DynamicOffsets.data();
    
    // 记录Offset
    dynOffsets[it->second.DynamicIndex] = (uint32)BufferView.offset;
}


VulkanMaterial::VulkanMaterial()
{
    
}
