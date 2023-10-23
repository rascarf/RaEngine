#pragma once
#include "Platform/Vulkan/VulkanCommonDefine.h"
#include "Platform/Vulkan/RayTracing/AccelerationStruct.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanTexture.h"


//单个SetLayout的信息
// 一个SetLayout会知道当前Set有什么BufferView，再去向Pool申请Set
class VulkanDescriptorSetLayoutInfo
{
private:
    typedef std::vector<VkDescriptorSetLayoutBinding> BindingsArray;

public:
    VulkanDescriptorSetLayoutInfo(){}

    ~VulkanDescriptorSetLayoutInfo(){}

public:
    int32           Set = -1; //Set的序号
    BindingsArray   Bindings; //保存在这个Set中的Descriptor
};

//CPU端持有Shader中所有的Set信息,指导产生真正SetLayout
class VulkanDescriptorSetLayoutsInfo
{
public:
    struct BindInfo
    {
        int32 Set;
        int32 Binding;
        uint32_t DynamicOffset;
        bool bDynamic;
    };

    VulkanDescriptorSetLayoutsInfo(){}

    ~VulkanDescriptorSetLayoutsInfo(){}

    VkDescriptorType GetDescriptorType(int32 Set,int32 Binding)
    {
        for(int32 i = 0; i < SetLayouts.size(); ++i)
        {
            if(SetLayouts[i].Set == Set)
            {
                for(int32 DescriptorIndex = 0 ; DescriptorIndex < SetLayouts[i].Bindings.size();DescriptorIndex++)
                {
                    if(SetLayouts[i].Bindings[DescriptorIndex].binding == Binding)
                    {
                        return SetLayouts[i].Bindings[DescriptorIndex].descriptorType;
                    }
                }
            }
        }

        return VK_DESCRIPTOR_TYPE_MAX_ENUM; 
    }

    void AddDescriptorSetLayoutBinding(const std::string VarName,int32 Set,VkDescriptorSetLayoutBinding binding)
    {
        VulkanDescriptorSetLayoutInfo* SetLayout = nullptr;

        for (int32 i = 0; i < SetLayouts.size(); ++i)
        {
            if (SetLayouts[i].Set == Set)
            {
                SetLayout = &(SetLayouts[i]);
                break;
            }
        }

        if (SetLayout == nullptr)
        {
            SetLayouts.push_back({ });
            SetLayout = &(SetLayouts[SetLayouts.size() - 1]);
        }

        for (int32 i = 0; i < SetLayout->Bindings.size(); ++i)
        {
            VkDescriptorSetLayoutBinding& setBinding = SetLayout->Bindings[i];
            if (setBinding.binding == binding.binding && setBinding.descriptorType == binding.descriptorType)
            {
                setBinding.stageFlags = setBinding.stageFlags | binding.stageFlags;
                return;
            }
        }

        SetLayout->Set = Set;
        SetLayout->Bindings.push_back(binding);

        // 保存参数的映射信息
        // 因为需要找到Set的序号，binding的位置，具体的Descriptor
        BindInfo paramInfo = {};
        paramInfo.Set      = Set;
        paramInfo.Binding  = binding.binding;
        ParamsMap.insert(std::make_pair(VarName, paramInfo));
    }

public:
    std::unordered_map<std::string,BindInfo> ParamsMap;
    std::vector<VulkanDescriptorSetLayoutInfo> SetLayouts;
};

struct VulkanAttribute
{
    VertexAttribute Attribute;
    int32           Location;
};

//Shader所产生的Set
//通过函数填写根据SetLayout产生的Set中的具体的Bind
//在Vulkan中不能直接操纵BufferView，而是只能绑定Set，所以有了BufferView之后更新Set
class VulkanDescriptorSet
{
public:
    ~VulkanDescriptorSet(){}
    VulkanDescriptorSet(){}

    void WriteImage(const std::string& name,Ref<VulkanTexture> Texture)
    {
        auto it = SetLayoutsInfo.ParamsMap.find(name);
        if (it == SetLayoutsInfo.ParamsMap.end())
        {
            RE_CORE_ERROR("Failed write buffer, {0} not found!", name.c_str());
            return;
        }

        auto BindInfo = it->second;

        VkWriteDescriptorSet writeDescriptorSet;
        ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
        writeDescriptorSet.dstSet          = DescriptorSets[BindInfo.Set];
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType  = SetLayoutsInfo.GetDescriptorType(BindInfo.Set, BindInfo.Binding);
        writeDescriptorSet.pBufferInfo     = nullptr;
        writeDescriptorSet.pImageInfo      = &(Texture->DescriptorInfo);
        writeDescriptorSet.dstBinding      = BindInfo.Binding;
        vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
    }

    void WriteBuffer(const std::string& name, const VkDescriptorBufferInfo* bufferInfo)
    {
        auto it = SetLayoutsInfo.ParamsMap.find(name);
        if (it == SetLayoutsInfo.ParamsMap.end())
        {
            RE_CORE_ERROR("Failed write buffer, {0} not found!", name.c_str());
            return;
        }

        auto bindInfo = it->second;

        VkWriteDescriptorSet writeDescriptorSet;
        ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
        writeDescriptorSet.dstSet          = DescriptorSets[bindInfo.Set];
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType  = SetLayoutsInfo.GetDescriptorType(bindInfo.Set, bindInfo.Binding);
        writeDescriptorSet.pBufferInfo     = bufferInfo;
        writeDescriptorSet.dstBinding      = bindInfo.Binding;
        vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
    }

    void WriteBuffer(const std::string& name, Ref<VulkanBuffer> buffer)
    {
        auto it = SetLayoutsInfo.ParamsMap.find(name);
        if (it == SetLayoutsInfo.ParamsMap.end())
        {
            RE_CORE_ERROR("Failed write buffer, {0} not found!", name.c_str());
            return;
        }

        auto bindInfo = it->second;

        VkWriteDescriptorSet writeDescriptorSet;
        ZeroVulkanStruct(writeDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
        writeDescriptorSet.dstSet          = DescriptorSets[bindInfo.Set];
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType  = SetLayoutsInfo.GetDescriptorType(bindInfo.Set, bindInfo.Binding);
        writeDescriptorSet.pBufferInfo     = &(buffer->Descriptor);
        writeDescriptorSet.dstBinding      = bindInfo.Binding;
        vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
    }

    void WriteStorageBufferArray(const std::string& name,std::vector<Ref<VulkanBuffer>> BufferArray)
    {
        auto it = SetLayoutsInfo.ParamsMap.find(name);
        if (it == SetLayoutsInfo.ParamsMap.end())
        {
            RE_CORE_ERROR("Failed write buffer, {0} not found!", name.c_str());
            return;
        }
        
        const auto bindInfo = it->second;

        std::vector<VkDescriptorBufferInfo> BufferInfos(BufferArray.size());
        for (int32 i = 0; i < BufferArray.size(); ++i)
        {
            BufferInfos[i].buffer = BufferArray[i]->Buffer;
            BufferInfos[i].offset = 0;
            BufferInfos[i].range = VK_WHOLE_SIZE;
        }

        VkWriteDescriptorSet WriteDescriptorSet;
        ZeroVulkanStruct(WriteDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
        WriteDescriptorSet.dstSet = DescriptorSets[bindInfo.Set];
        WriteDescriptorSet.dstBinding = bindInfo.Binding;
        WriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        WriteDescriptorSet.descriptorCount = BufferInfos.size();
        WriteDescriptorSet.pBufferInfo = BufferInfos.data();
        
        vkUpdateDescriptorSets(device, 1, &WriteDescriptorSet, 0, nullptr);
    }

    void WriteTextureArray(const std::string& name,std::vector<Ref<VulkanTexture>> TextureArray)
    {
        auto it = SetLayoutsInfo.ParamsMap.find(name);
        if (it == SetLayoutsInfo.ParamsMap.end())
        {
            RE_CORE_ERROR("Failed write buffer, {0} not found!", name.c_str());
            return;
        }

        const auto bindInfo = it->second;
        
        std::vector<VkDescriptorImageInfo> textureImageInfos(TextureArray.size());
        for (int32 i = 0; i < TextureArray.size(); ++i)
        {
            textureImageInfos[i] = TextureArray[i]->DescriptorInfo;
        }

        VkWriteDescriptorSet textureWriteDescriptorSet;
        ZeroVulkanStruct(textureWriteDescriptorSet, VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);
        textureWriteDescriptorSet.dstSet = DescriptorSets[bindInfo.Set];
        textureWriteDescriptorSet.dstBinding = bindInfo.Binding;
        textureWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textureWriteDescriptorSet.descriptorCount = textureImageInfos.size();
        textureWriteDescriptorSet.pImageInfo = textureImageInfos.data();
        
        vkUpdateDescriptorSets(device, 1, &textureWriteDescriptorSet, 0, nullptr);
    }

    void WriteAccelerationStruct(const std::string& name,const VkAccelerationStructureKHR& accel)
    {
        auto it = SetLayoutsInfo.ParamsMap.find(name);
        if (it == SetLayoutsInfo.ParamsMap.end())
        {
            RE_CORE_ERROR("Failed write buffer, {0} not found!", name.c_str());
            return;
        }

        auto bindInfo = it->second;
        
        VkWriteDescriptorSetAccelerationStructureKHR tlas_info = {};
        tlas_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
        tlas_info.accelerationStructureCount = 1;
        tlas_info.pAccelerationStructures = &accel;

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = DescriptorSets[bindInfo.Set];
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        writeDescriptorSet.dstBinding = bindInfo.Binding;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.dstArrayElement = 0;
        writeDescriptorSet.pNext = &tlas_info;

        vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
    }

    void WriteBindOffset(const std::string& name,uint32_t Offset)
    {
        auto it = SetLayoutsInfo.ParamsMap.find(name);
        if (it == SetLayoutsInfo.ParamsMap.end())
        {
            RE_CORE_ERROR("Failed write buffer, {0} not found!", name.c_str());
        }

        auto& bindInfo = it->second;
        bindInfo.DynamicOffset = Offset;
        bindInfo.bDynamic = true;
    }

    void BindSet(VkCommandBuffer& Cmd, VkPipelineLayout PipelineLayout)
    {
        //检查有多少个Set的DynamicOffset是一致的
        //TODO 一起绑定
        
        //依次绑定不同DynamicOffset的Set
        for(int32 i = 0; i < SetLayoutsInfo.SetLayouts.size(); ++i)
        {
            std::vector<uint32_t> Offsets;
            for(int32 DescriptorIndex = 0 ; DescriptorIndex < SetLayoutsInfo.SetLayouts[i].Bindings.size();DescriptorIndex++)
            {
                for(auto BindInfoPair : SetLayoutsInfo.ParamsMap)
                {
                    auto BindInfo = BindInfoPair.second;

                    if(BindInfo.bDynamic == false)
                        continue;
                    
                    if(BindInfo.Binding == DescriptorIndex && BindInfo.Set == i)
                    {
                        Offsets.push_back(BindInfo.DynamicOffset);
                    }
                }
            }
            
            vkCmdBindDescriptorSets(Cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 0, 1, &DescriptorSets[i], Offsets.size(), Offsets.data());
        }
    }

public:
    VkDevice    device;

    //这两者一个是描述，一个是真实的GPU上的Sets
    VulkanDescriptorSetLayoutsInfo     SetLayoutsInfo;
    std::vector<VkDescriptorSet>    DescriptorSets;
};
