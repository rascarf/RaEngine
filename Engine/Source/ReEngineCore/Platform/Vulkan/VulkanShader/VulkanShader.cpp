#include "VulkanShader.h"
#include "spirv_cross.hpp"
#include "Platform/Vulkan/VulkanBuffers/VulkanVertexBuffer.h"

VulkanShader::~VulkanShader()
{
    if (vertShaderModule)
    {
        vertShaderModule.reset();
        vertShaderModule = nullptr;
    }

    if (fragShaderModule)
    {
        fragShaderModule.reset();
        fragShaderModule = nullptr;
    }

    if (geomShaderModule)
    {
        geomShaderModule.reset();
        geomShaderModule = nullptr;
    }

    if (compShaderModule)
    {
        compShaderModule.reset();
        compShaderModule = nullptr;
    }

    if (tescShaderModule)
    {
        tescShaderModule.reset();
        tescShaderModule = nullptr;
    }

    if (teseShaderModule)
    {
        teseShaderModule.reset();
        teseShaderModule = nullptr;
    }
    
    for (int32 i = 0; i < descriptorSetLayouts.size(); ++i)
    {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayouts[i], VULKAN_CPU_ALLOCATOR);
    }
    descriptorSetLayouts.clear();

    if (pipelineLayout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device, pipelineLayout, VULKAN_CPU_ALLOCATOR);
        pipelineLayout = VK_NULL_HANDLE;
    }

    for (int32 i = 0; i < descriptorSetPools.size(); ++i)
    {
        descriptorSetPools[i].reset();
    }
    descriptorSetPools.clear();
}

Ref<VulkanShader> VulkanShader::Create(Ref<VulkanDevice> vulkanDevice,bool DynamicUBO
                                       ,const std::vector<unsigned char>* Vert,const std::vector<unsigned char>* Frag
                                       ,const std::vector<unsigned char>* Geom,const std::vector<unsigned char>* Compute
                                       ,const std::vector<unsigned char>* Tesc,const std::vector<unsigned char>* Tese)
{
    using ShaderModuleRef = Ref<VulkanShaderModule>;
    
    ShaderModuleRef vertModule = Vert ? VulkanShaderModule::Create(vulkanDevice, *Vert, VK_SHADER_STAGE_VERTEX_BIT)   : nullptr;
    ShaderModuleRef fragModule = Frag ? VulkanShaderModule::Create(vulkanDevice, *Frag, VK_SHADER_STAGE_FRAGMENT_BIT) : nullptr;
    ShaderModuleRef geomModule = Geom ? VulkanShaderModule::Create(vulkanDevice, *Geom, VK_SHADER_STAGE_GEOMETRY_BIT) : nullptr;
    ShaderModuleRef compModule = Compute ? VulkanShaderModule::Create(vulkanDevice, *Compute, VK_SHADER_STAGE_COMPUTE_BIT) : nullptr;
    ShaderModuleRef tescModule = Tesc ? VulkanShaderModule::Create(vulkanDevice, *Tesc, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)    : nullptr;
    ShaderModuleRef teseModule = Tese ? VulkanShaderModule::Create(vulkanDevice, *Tese, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) : nullptr;

    Ref<VulkanShader> Shader = CreateRef<VulkanShader>();

    Shader->device = vulkanDevice->GetInstanceHandle();
    Shader->dynamicUBO = DynamicUBO;
    Shader->vertShaderModule = vertModule;
    Shader->fragShaderModule = fragModule;
    Shader->geomShaderModule = geomModule;
    Shader->compShaderModule = compModule;
    Shader->tescShaderModule = tescModule;
    Shader->teseShaderModule = teseModule;

    Shader->Compile();

    return Shader;
}

Ref<VulkanShader> VulkanShader::CreateCompute(Ref<VulkanDevice> vulkanDevice, bool DynamicUBO,const std::vector<unsigned char>* Compute)
{
    using ShaderModuleRef = Ref<VulkanShaderModule>;

    Ref<VulkanShader> Shader = CreateRef<VulkanShader>();
    ShaderModuleRef compModule = Compute ? VulkanShaderModule::Create(vulkanDevice, *Compute, VK_SHADER_STAGE_COMPUTE_BIT) : nullptr;

    Shader->device = vulkanDevice->GetInstanceHandle();
    Shader->dynamicUBO = DynamicUBO;
    Shader->compShaderModule = compModule;
    Shader->Compile();

    return Shader;
}

void VulkanShader::Compile()
{
    ProcessShaderModule(vertShaderModule);
    ProcessShaderModule(fragShaderModule);
    ProcessShaderModule(geomShaderModule);
    ProcessShaderModule(compShaderModule);
    ProcessShaderModule(tescShaderModule);
    ProcessShaderModule(teseShaderModule);

    GenerateInputInfo();
    GenerateLayout();
}

void VulkanShader::ProcessShaderModule(Ref<VulkanShaderModule> ShaderModule)
{
    if(!ShaderModule)
    {
        return;
    }

    VkPipelineShaderStageCreateInfo ShaderCreateInfo;
    ZeroVulkanStruct(ShaderCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
    ShaderCreateInfo.stage = ShaderModule->mShaderStage;
    ShaderCreateInfo.module = ShaderModule->Handle;
    ShaderCreateInfo.pName = "main"; //TODO 这里思考下如何拓展
    ShaderStageInfos.push_back(ShaderCreateInfo);

    spirv_cross::Compiler Compiler((uint32*)ShaderModule->Code->data(), ShaderModule->Code->size() / sizeof(uint32_t));
    spirv_cross::ShaderResources Resources = Compiler.get_shader_resources();

    ProcessAttachment(Compiler, Resources, ShaderModule->mShaderStage);
    ProcessUniformBuffers(Compiler, Resources, ShaderModule->mShaderStage);
    ProcessTextures(Compiler, Resources, ShaderModule->mShaderStage);
    ProcessStorageImages(Compiler, Resources, ShaderModule->mShaderStage);
    ProcessInput(Compiler, Resources, ShaderModule->mShaderStage);
    ProcessStorageBuffers(Compiler, Resources, ShaderModule->mShaderStage);
}

void VulkanShader::ProcessAttachment(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources,VkPipelineStageFlags stageFlags)
{
    for(int32 i = 0; i < resources.subpass_inputs.size(); ++i)
    {
        spirv_cross::Resource& res      = resources.subpass_inputs[i];
        spirv_cross::SPIRType type      = compiler.get_type(res.type_id);
        spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
        const std::string &varName      = compiler.get_name(res.id);

        int32 set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        int32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);

        VkDescriptorSetLayoutBinding setLayoutBinding = {};
        setLayoutBinding.binding             = binding;
        setLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        setLayoutBinding.descriptorCount    = 1;
        setLayoutBinding.stageFlags         = stageFlags;
        setLayoutBinding.pImmutableSamplers = nullptr;
        
        SetLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);
        auto it = imageParams.find(varName);
        if(it == imageParams.end())
        {
            ImageInfo ImageInfo = {};
            ImageInfo.Set = set;
            ImageInfo.Binding = binding;
            ImageInfo.StageFlags = stageFlags;
            ImageInfo.DescriptorType = setLayoutBinding.descriptorType;
            imageParams.insert(std::make_pair(varName, ImageInfo));
        }
        else
        {
            it->second.StageFlags |= stageFlags;
        }
    }
}

void VulkanShader::ProcessUniformBuffers(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources,VkShaderStageFlags stageFlags)
{

    for (int32 i = 0; i < resources.uniform_buffers.size(); ++i)
    {
        spirv_cross::Resource& res      = resources.uniform_buffers[i];
        spirv_cross::SPIRType type      = compiler.get_type(res.type_id);
        spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
        const std::string &varName      = compiler.get_name(res.id);
        const std::string &typeName     = compiler.get_name(res.base_type_id);
        uint32 uniformBufferStructSize  = (uint32)compiler.get_declared_struct_size(type);

        int32 set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        int32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);

        // [layout (binding = 0) uniform MVPDynamicBlock] 标记为Dynamic的buffer
        VkDescriptorSetLayoutBinding setLayoutBinding = {};
        setLayoutBinding.binding             = binding;
        setLayoutBinding.descriptorType     = (typeName.find("Dynamic") != std::string::npos || dynamicUBO) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        setLayoutBinding.descriptorCount    = 1;
        setLayoutBinding.stageFlags         = stageFlags;
        setLayoutBinding.pImmutableSamplers = nullptr;

        SetLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);

        // 保存UBO变量信息
        auto it = bufferParams.find(varName);
        if (it == bufferParams.end())
        {
            BufferInfo bufferInfo = {};
            bufferInfo.Set            = set;
            bufferInfo.Binding        = binding;
            bufferInfo.BufferSize     = uniformBufferStructSize;
            bufferInfo.StageFlags     = stageFlags;
            bufferInfo.DescriptorType = setLayoutBinding.descriptorType;
            bufferParams.insert(std::make_pair(varName, bufferInfo));
        }
        else
        {
            it->second.StageFlags |= setLayoutBinding.stageFlags;
        }
    }
    
}

void VulkanShader::ProcessTextures(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources,VkShaderStageFlags stageFlags)
{
    for (int32 i = 0; i < resources.sampled_images.size(); ++i)
    {
        spirv_cross::Resource& res      = resources.sampled_images[i];
        spirv_cross::SPIRType type      = compiler.get_type(res.type_id);
        spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
        const std::string&      varName = compiler.get_name(res.id);

        int32 set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        int32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);

        VkDescriptorSetLayoutBinding setLayoutBinding = {};
        setLayoutBinding.binding             = binding;
        setLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        setLayoutBinding.descriptorCount    = 1;
        setLayoutBinding.stageFlags         = stageFlags;
        setLayoutBinding.pImmutableSamplers = nullptr;

        SetLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);

        auto it = imageParams.find(varName);
        if (it == imageParams.end())
        {
            ImageInfo imageInfo = {};
            imageInfo.Set            = set;
            imageInfo.Binding        = binding;
            imageInfo.StageFlags     = stageFlags;
            imageInfo.DescriptorType = setLayoutBinding.descriptorType;
            imageParams.insert(std::make_pair(varName, imageInfo));
        }
        else
        {
            it->second.StageFlags |= stageFlags;
        }
    }
}

void VulkanShader::ProcessInput(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources,VkShaderStageFlags stageFlags)
{
    if (stageFlags != VK_SHADER_STAGE_VERTEX_BIT)
    {
        return;
    }

    for (int32 i = 0; i < resources.stage_inputs.size(); ++i)
    {
        spirv_cross::Resource& res = resources.stage_inputs[i];
        spirv_cross::SPIRType type = compiler.get_type(res.type_id);
        const std::string &varName = compiler.get_name(res.id);
        int32 inputAttributeSize   = type.vecsize;

        VertexAttribute attribute  = StringToVertexAttribute(varName.c_str());
        if (attribute == VertexAttribute::VA_None)
        {
            if (inputAttributeSize == 1)
            {
                attribute = VertexAttribute::VA_InstanceFloat1;
            }
            else if (inputAttributeSize == 2)
            {
                attribute = VertexAttribute::VA_InstanceFloat2;
            }
            else if (inputAttributeSize == 3)
            {
                attribute = VertexAttribute::VA_InstanceFloat3;
            }
            else if (inputAttributeSize == 4)
            {
                attribute = VertexAttribute::VA_InstanceFloat4;
            }
            
            RE_CORE_ERROR("Not found attribute : {0}, treat as instance attribute : {1}.", varName.c_str(), int32(attribute));
        }

        // location必须连续
        int32 location = compiler.get_decoration(res.id, spv::DecorationLocation);
        VulkanAttribute Attribute = {};
        Attribute.Location  = location;
        Attribute.Attribute = attribute;
        m_InputAttributes.push_back(Attribute);
    }
}

void VulkanShader::ProcessStorageBuffers(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources,VkShaderStageFlags stageFlags)
{
    for (int32 i = 0; i < resources.storage_buffers.size(); ++i)
    {
        spirv_cross::Resource& res      = resources.storage_buffers[i];
        spirv_cross::SPIRType type      = compiler.get_type(res.type_id);
        spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
        const std::string &varName      = compiler.get_name(res.id);

        int32 set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        int32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);

        VkDescriptorSetLayoutBinding setLayoutBinding = {};
        setLayoutBinding.binding            = binding;
        setLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        setLayoutBinding.descriptorCount    = 1;
        setLayoutBinding.stageFlags         = stageFlags;
        setLayoutBinding.pImmutableSamplers = nullptr;

        SetLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);

        // 保存UBO变量信息
        auto it = bufferParams.find(varName);
        if (it == bufferParams.end())
        {
            BufferInfo bufferInfo = {};
            bufferInfo.Set            = set;
            bufferInfo.Binding        = binding;
            bufferInfo.BufferSize     = 0;
            bufferInfo.StageFlags     = stageFlags;
            bufferInfo.DescriptorType = setLayoutBinding.descriptorType;
            bufferParams.insert(std::make_pair(varName, bufferInfo));
        }
        else
        {
            it->second.StageFlags = it->second.StageFlags | setLayoutBinding.stageFlags;
        }
    }
}

void VulkanShader::ProcessStorageImages(spirv_cross::Compiler& compiler, spirv_cross::ShaderResources& resources,VkShaderStageFlags stageFlags)
{
    for (int32 i = 0; i < resources.storage_images.size(); ++i)
    {
        spirv_cross::Resource& res      = resources.storage_images[i];
        spirv_cross::SPIRType type      = compiler.get_type(res.type_id);
        spirv_cross::SPIRType base_type = compiler.get_type(res.base_type_id);
        const std::string&      varName = compiler.get_name(res.id);

        int32 set     = compiler.get_decoration(res.id, spv::DecorationDescriptorSet);
        int32 binding = compiler.get_decoration(res.id, spv::DecorationBinding);

        VkDescriptorSetLayoutBinding setLayoutBinding = {};
        setLayoutBinding.binding             = binding;
        setLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        setLayoutBinding.descriptorCount    = 1;
        setLayoutBinding.stageFlags         = stageFlags;
        setLayoutBinding.pImmutableSamplers = nullptr;

        SetLayoutsInfo.AddDescriptorSetLayoutBinding(varName, set, setLayoutBinding);

        auto it = imageParams.find(varName);
        if (it == imageParams.end())
        {
            ImageInfo imageInfo = {};
            imageInfo.Set            = set;
            imageInfo.Binding        = binding;
            imageInfo.StageFlags     = stageFlags;
            imageInfo.DescriptorType = setLayoutBinding.descriptorType;
            imageParams.insert(std::make_pair(varName, imageInfo));
        }
        else
        {
            it->second.StageFlags |= stageFlags;
        }

    }
}

void VulkanShader::GenerateInputInfo()
{
    // 对inputAttributes进行排序，获取Attributes列表
    std::sort(
        m_InputAttributes.begin(),
        m_InputAttributes.end(),
        [](const VulkanAttribute& a, const VulkanAttribute& b) -> bool
        {
            return a.Location < b.Location;
        }
    );

    // 对inputAttributes进行归类整理
    for (int32 i = 0; i < m_InputAttributes.size(); ++i)
    {
        VertexAttribute attribute = m_InputAttributes[i].Attribute;
        if (attribute == VertexAttribute::VA_InstanceFloat1 || attribute == VertexAttribute::VA_InstanceFloat2 || attribute == VertexAttribute::VA_InstanceFloat3 || attribute == VertexAttribute::VA_InstanceFloat4)
        {
            instancesAttributes.push_back(attribute);
        }
        else
        {
            perVertexAttributes.push_back(attribute);
        }
    }

    inputBindings.resize(0);
    if (perVertexAttributes.size() > 0)
    {
        int32 stride = 0;
        for (int32 i = 0; i < perVertexAttributes.size(); ++i)
        {
            stride += VertexAttributeToSize(perVertexAttributes[i]);
        }
            
        VkVertexInputBindingDescription perVertexInputBinding = {};
        perVertexInputBinding.binding   = 0;
        perVertexInputBinding.stride    = stride;
        perVertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        inputBindings.push_back(perVertexInputBinding);
    }

    if (instancesAttributes.size() > 0)
    {
        int32 stride = 0;
        for (int32 i = 0; i < instancesAttributes.size(); ++i)
        {
            stride += VertexAttributeToSize(instancesAttributes[i]);
        }
            
        VkVertexInputBindingDescription instanceInputBinding = {};
        instanceInputBinding.binding   = 1;
        instanceInputBinding.stride    = stride;
        instanceInputBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        inputBindings.push_back(instanceInputBinding);
    }

    int location = 0;
    if (perVertexAttributes.size() > 0)
    {
        int32 offset = 0;
        for (int32 i = 0; i < perVertexAttributes.size(); ++i)
        {
            VkVertexInputAttributeDescription inputAttribute = {};
            inputAttribute.binding  = 0;
            inputAttribute.location = location;
            inputAttribute.format   = VertexAttributeToVkFormat(perVertexAttributes[i]);
            inputAttribute.offset   = offset;
            offset += VertexAttributeToSize(perVertexAttributes[i]);
            inputAttributes.push_back(inputAttribute);

            location += 1;
        }
    }

    if (instancesAttributes.size() > 0)
    {
        int32 offset = 0;
        for (int32 i = 0; i < instancesAttributes.size(); ++i)
        {
            VkVertexInputAttributeDescription inputAttribute = {};
            inputAttribute.binding  = 1;
            inputAttribute.location = location;
            inputAttribute.format   = VertexAttributeToVkFormat(instancesAttributes[i]);
            inputAttribute.offset   = offset;
            offset += VertexAttributeToSize(instancesAttributes[i]);
            inputAttributes.push_back(inputAttribute);

            location += 1;
        }
    }
}

void VulkanShader::GenerateLayout()
{
    std::vector<VulkanDescriptorSetLayoutInfo>& setLayouts = SetLayoutsInfo.SetLayouts;

    // 先按照set进行排序
    std::sort(
        setLayouts.begin(),
        setLayouts.end(),
        [](const VulkanDescriptorSetLayoutInfo& a, const VulkanDescriptorSetLayoutInfo& b) -> bool
        {
            return a.Set < b.Set;
        }
    );

    // 再按照binding进行排序
    for (int32 i = 0; i < setLayouts.size(); ++i)
    {
        std::vector<VkDescriptorSetLayoutBinding>& bindings = setLayouts[i].Bindings;
        std::sort(
            bindings.begin(),
            bindings.end(),
            [](const VkDescriptorSetLayoutBinding& a, const VkDescriptorSetLayoutBinding& b) -> bool
            {
                return a.binding < b.binding;
            }
        );
    }

    for (int32 i = 0; i < SetLayoutsInfo.SetLayouts.size(); ++i)
    {
        VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
        VulkanDescriptorSetLayoutInfo& setLayoutInfo = SetLayoutsInfo.SetLayouts[i];

        VkDescriptorSetLayoutCreateInfo descSetLayoutInfo;
        ZeroVulkanStruct(descSetLayoutInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
        descSetLayoutInfo.bindingCount = (uint32_t)setLayoutInfo.Bindings.size();
        descSetLayoutInfo.pBindings    = setLayoutInfo.Bindings.data();
        descSetLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

        VkDescriptorBindingFlags bindless_flags =VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
        std::vector<VkDescriptorBindingFlags> binding_flags;
        for(int32 i = 0 ; i < (uint32_t)setLayoutInfo.Bindings.size() ; i++)
        {
            if(setLayoutInfo.Bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE|| setLayoutInfo.Bindings[i].descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            {
                binding_flags.push_back(bindless_flags);
            }
            else
            {
                binding_flags.push_back(0);
            }
        }
        
        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info
        {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT, nullptr
        };
        
        extended_info.bindingCount = (uint32_t)setLayoutInfo.Bindings.size();
        extended_info.pBindingFlags = binding_flags.data();
        descSetLayoutInfo.pNext = &extended_info;
        
        VERIFYVULKANRESULT(vkCreateDescriptorSetLayout(device, &descSetLayoutInfo, VULKAN_CPU_ALLOCATOR, &descriptorSetLayout)); 

        descriptorSetLayouts.push_back(descriptorSetLayout);
    }

    VkPipelineLayoutCreateInfo pipeLayoutInfo;
    ZeroVulkanStruct(pipeLayoutInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
    pipeLayoutInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
    pipeLayoutInfo.pSetLayouts    = descriptorSetLayouts.data();
    VERIFYVULKANRESULT(vkCreatePipelineLayout(device, &pipeLayoutInfo, VULKAN_CPU_ALLOCATOR, &pipelineLayout));
}

Ref<VulkanDescriptorSet> VulkanShader::AllocateDescriptorSet()
{
    //Shader反射的Set参数为0
    if(SetLayoutsInfo.SetLayouts.size() == 0)
    {
        return nullptr;
    }

    Ref<VulkanDescriptorSet> OutSet = CreateRef<VulkanDescriptorSet>();
    OutSet->device = device;
    OutSet->SetLayoutsInfo = SetLayoutsInfo;
    OutSet->DescriptorSets.resize(SetLayoutsInfo.SetLayouts.size());

    for (int32 i = (int32)descriptorSetPools.size() - 1; i >= 0; --i)
    {
        if (descriptorSetPools[i]->AllocateDescriptorSet(OutSet->DescriptorSets.data()))
        {
            return OutSet;
        }
    }

    Ref<VulkanShaderDescriptorPool> setPool = CreateRef<VulkanShaderDescriptorPool>(device, 64, SetLayoutsInfo, descriptorSetLayouts);

    descriptorSetPools.push_back(setPool);

    if(setPool->AllocateDescriptorSet(OutSet->DescriptorSets.data()))
    {
        return OutSet;
    }
}
