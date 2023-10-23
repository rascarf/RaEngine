#pragma once
#include "spirv_cross.hpp"
#include "VulkanDescriptorAllocator.h"
#include "VulkanShaderModule.h"
#include "Platform/Vulkan/VulkanCommonDefine.h"
#include "VulkanDescriptorSet.h"


//ShaderModule之间是可以共享数据的
class VulkanShader
{
public:

    ~VulkanShader();
    
    struct BufferInfo
    {
        uint32 Set = 0;
        uint32 Binding = 0;
        uint32 BufferSize = 0;
        VkDescriptorType DescriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        VkShaderStageFlags StageFlags = 0;
    };

    struct ImageInfo
    {
        uint32 Set = 0;
        uint32 Binding = 0;
        VkDescriptorType DescriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        VkShaderStageFlags StageFlags = 0;
    };

    struct ASInfo
    {
        uint32 Set = 0;
        uint32 Binding = 0;
        VkDescriptorType DescriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        VkShaderStageFlags StageFlags = 0;
    };
    
    static Ref<VulkanShader> Create(Ref<VulkanDevice> vulkanDevice,bool DynamicUBO,const std::vector<unsigned char>* Vert,const std::vector<unsigned char>* Frag,const std::vector<unsigned char>* Geom,const std::vector<unsigned char>* Compute,const std::vector<unsigned char>* Tesc,const std::vector<unsigned char>* Tese);
    static Ref<VulkanShader> CreateCompute(Ref<VulkanDevice> vulkanDevice,bool DynamicUBO,const std::vector<unsigned char>* Compute);
    static Ref<VulkanShader> CreateRayTracingShader(Ref<VulkanDevice> vulkanDevice,const std::vector<unsigned char>* ClosethitShaderModule,const std::vector<unsigned char>* RayGenerationShaderModule,const std::vector<unsigned char>* MissShaderModule);
    static Ref<VulkanShader> CreateRayTracingShader(Ref<VulkanDevice> vulkanDevice,const std::vector<unsigned char>* ClosethitShaderModule,const std::vector<unsigned char>* RayGenerationShaderModule,const std::vector<unsigned char>* MissShaderModule,const std::unordered_map<std::string,int>& ArrayParams);
    
    void Compile();
    virtual void ProcessShaderModule(Ref<VulkanShaderModule> ShaderModule);
    virtual void ProcessRayTracingShaderModule(Ref<VulkanShaderModule> ShaderModule);
    virtual void ProcessAttachment(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkPipelineStageFlags stageFlags);

    virtual void ProcessUniformBuffers(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags);
    virtual void ProcessTextures(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags);

    virtual void ProcessInput(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags);
    virtual void ProcessStorageBuffers(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags);
    virtual void ProcessStorageImages(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags);

    virtual void ProcessAccelerationSructure(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources, VkShaderStageFlags stageFlags);
    
    virtual void GenerateInputInfo();
    virtual void GenerateLayout();

    Ref<VulkanDescriptorSet> AllocateDescriptorSet();
    
public:
    Ref<VulkanShaderModule>           vertShaderModule = nullptr;
    Ref<VulkanShaderModule>           fragShaderModule = nullptr;
    Ref<VulkanShaderModule>           geomShaderModule = nullptr;
    Ref<VulkanShaderModule>           compShaderModule = nullptr;
    Ref<VulkanShaderModule>           tescShaderModule = nullptr;
    Ref<VulkanShaderModule>           teseShaderModule = nullptr;

//------------------------------RayTracing------------------------------------
    Ref<VulkanShaderModule>           ClosethitShaderModule = nullptr;
    Ref<VulkanShaderModule>           RayGenerationShaderModule = nullptr;
    Ref<VulkanShaderModule>           MissShaderModule = nullptr;
    bool                              bIsRayTracingShader;
public:

//------------------------------CPU------------------------------------
    bool dynamicUBO = false;

    std::vector<VkPipelineShaderStageCreateInfo> ShaderStageInfos;
    
    //反射出来的所有的Set信息
    VulkanDescriptorSetLayoutsInfo     SetLayoutsInfo;

    //Param信息
    std::unordered_map<std::string, BufferInfo> bufferParams;
    std::unordered_map<std::string, ImageInfo>  imageParams;
    std::unordered_map<std::string,ASInfo>      asParams;

    //Location信息
    std::vector<VulkanAttribute> m_InputAttributes;

    //Attr信息
    std::vector<VertexAttribute>    instancesAttributes;
    std::vector<VertexAttribute>    perVertexAttributes;

    std::unordered_map<std::string,int32>   ArrayParams;

//------------------------------GPU------------------------------------
    
    std::vector<VkVertexInputBindingDescription>             inputBindings;
    std::vector<VkVertexInputAttributeDescription>           inputAttributes;

    std::vector<VkDescriptorSetLayout>                      descriptorSetLayouts;

    std::vector<Ref<VulkanShaderDescriptorPool>>        descriptorSetPools;
    VkPipelineLayout                pipelineLayout = VK_NULL_HANDLE;
    VkDevice    device;
};



