#pragma once
#include "Platform/Vulkan/VulkanCommonDefine.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanTexture.h"

//单个SetLayout的信息
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

public:
    VkDevice    device;

    //这两者一个是描述，一个是真实的GPU上的Sets
    VulkanDescriptorSetLayoutsInfo     SetLayoutsInfo;
    std::vector<VkDescriptorSet>    DescriptorSets; 
};
