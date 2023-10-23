#include "SBTWrapper.h"

void SBTWrapper::Setup(VulkanContext* ctx, uint32 FamilyIndex,const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& RtProps)
{
    mCtx = ctx;
    QueueIndex = FamilyIndex;
    HandleSize = RtProps.shaderGroupHandleSize;
    HandleAlignment = RtProps.shaderGroupHandleAlignment;
}

void SBTWrapper::Destroy()
{
    for(auto& buffer : mBuffer) 
    {
        buffer.reset();
    }

    for(auto& i : mIndex)
        i = {};

    
}

void SBTWrapper::Create(VkPipeline RtPipeline, VkRayTracingPipelineCreateInfoKHR Pipeline_Info,const std::vector<VkRayTracingPipelineCreateInfoKHR>& CreateInfos)
{
    for(auto& b : mBuffer)
    {
        b.reset();
    }

    uint32_t TotalGroupCount{0};
    std::vector<uint32_t> GroupCountPerInput;
    GroupCountPerInput.reserve(1 + CreateInfos.size());
    if(Pipeline_Info.sType == VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR)
    {
        AddIndices(Pipeline_Info,CreateInfos);
        GroupCountPerInput.push_back(Pipeline_Info.groupCount);
        TotalGroupCount += Pipeline_Info.groupCount;

        for(const auto& lib : CreateInfos)
        {
            GroupCountPerInput.push_back(lib.groupCount);
            TotalGroupCount += lib.groupCount;
        }
    }
    else
    {
        for(auto& i : mIndex)
        {
            if(!i.empty())
            {
                TotalGroupCount = std::max(TotalGroupCount, *std::max_element(std::begin(i), std::end(i)));
            }
            TotalGroupCount++;
            GroupCountPerInput.push_back(TotalGroupCount);
        }
    }

    uint32 SbtSize = TotalGroupCount * HandleSize;
    std::vector<uint8_t> shader_handle_storage(SbtSize);

    auto result = vkGetRayTracingShaderGroupHandlesKHR(
        mCtx->Instance->GetDevice()->GetInstanceHandle(),
        RtPipeline,
        0,
        TotalGroupCount,
        SbtSize,
        shader_handle_storage.data()
        );

    //取当前类型ShaderRecord中最大的stride
    auto FindStride = [&](auto entry,auto& stride)
    {
        stride = align_up(HandleSize,HandleAlignment);
        for(auto& e : entry)
        {
            uint32_t dataHandleSize =align_up(static_cast<uint32_t>(HandleSize + e.second.size() * sizeof(uint8_t)), HandleAlignment);
            stride = std::max(stride,dataHandleSize);
        }
    };

    FindStride(mData[RayGen], mStride[RayGen]);
    FindStride(mData[Miss], mStride[Miss]);
    FindStride(mData[Hit], mStride[Hit]);
    FindStride(mData[Callable], mStride[Callable]);

    // Handler-Data这样的自定义结构
    std::array<std::vector<uint8_t>,4> Stage;
    Stage[RayGen] = std::vector<uint8_t>(mStride[RayGen] * index_count(RayGen));
    Stage[Miss] = std::vector<uint8_t>(mStride[Miss] * index_count(Miss));
    Stage[Hit] = std::vector<uint8_t>(mStride[Hit] * index_count(Hit));
    Stage[Callable] = std::vector<uint8_t>(mStride[Callable] * index_count(Callable));

    auto CopyHandles = [&](std::vector<uint8_t>& Buffer,std::vector<uint32_t>& Indices,uint32_t Stride,auto& Data)
    {
        auto* pBuffer = Buffer.data();
        for(uint32_t Index = 0; Index < static_cast<uint32_t>(Indices.size()); Index++)
        {
            auto* pStart = pBuffer;
            memcpy(pBuffer,shader_handle_storage.data() + (Indices[Index] * HandleSize),HandleSize);

            auto it = Data.find(Index);
            if(it != std::end(Data))
            {
                pBuffer += HandleSize;
                memcpy(pBuffer,it->second.data(),it->second.size() * sizeof(uint8_t));
            }

            pBuffer = pStart + Stride;
        }
    };

    CopyHandles(Stage[RayGen],mIndex[RayGen],mStride[RayGen],mData[RayGen]);
    CopyHandles(Stage[Miss],mIndex[Miss],mStride[Miss],mData[Miss]);
    CopyHandles(Stage[Hit],mIndex[Hit],mStride[Hit],mData[Hit]);
    CopyHandles(Stage[Callable],mIndex[Callable],mStride[Callable],mData[Callable]);

    auto Usage_Flags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR ;
    auto mem_flag = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    for(uint32_t i = 0 ; i < 4 ; i++)
    {
        if(!Stage[i].empty())
        {
            mBuffer[i] = VulkanBuffer::CreateBuffer(
                    mCtx->GetVulkanInstance()->GetDevice(),
                    Usage_Flags,
                    mem_flag,
                    Stage[i].size(),
                    Stage[i].data()
                );
        }
    }
}

void SBTWrapper::AddIndices(VkRayTracingPipelineCreateInfoKHR pipeline_info,const std::vector<VkRayTracingPipelineCreateInfoKHR>& create_infos)
{
    for(auto& i : mIndex)
        i = {};

    uint32_t StageIndex = 0;
    for(size_t i = 0 ; i < create_infos.size() + 1; i++)
    {
        const auto& info = (i == 0) ? pipeline_info : create_infos[i - 1];
        uint32_t GroupOffset = StageIndex;

        for(uint32_t g = 0 ; g < info.groupCount ; g++)
        {
            if(info.pGroups[g].type == VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR)
            {
                if(info.pStages[StageIndex].stage == VK_SHADER_STAGE_RAYGEN_BIT_KHR)
                {
                    mIndex[RayGen].push_back(g + GroupOffset);
                    StageIndex++;
                }
                else if(info.pStages[StageIndex].stage == VK_SHADER_STAGE_MISS_BIT_KHR)
                {
                    mIndex[Miss].push_back(g + GroupOffset);
                    StageIndex++;
                }
                else if(info.pStages[StageIndex].stage == VK_SHADER_STAGE_CALLABLE_BIT_KHR)
                {
                    mIndex[Callable].push_back(g + GroupOffset);
                    StageIndex++;
                }
            }
            else
            {
                mIndex[Hit].push_back(g + GroupOffset);
                if(info.pGroups[g].closestHitShader != VK_SHADER_UNUSED_KHR) StageIndex++;
                if(info.pGroups[g].anyHitShader != VK_SHADER_UNUSED_KHR) StageIndex++;
                if(info.pGroups[g].intersectionShader != VK_SHADER_UNUSED_KHR) StageIndex++;
            }
        }
    }
}

VkDeviceAddress SBTWrapper::get_address(GroupType t)
{
    if(!mBuffer[t])
    {
        return 0;
    }
    
    if(!mBuffer[t]->Size)
    {
        return 0;
    }

    VkBufferDeviceAddressInfo i{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, mBuffer[t]->Buffer};
    return vkGetBufferDeviceAddress(mCtx->Instance->GetDevice()->GetInstanceHandle(),&i);
}

const VkStridedDeviceAddressRegionKHR SBTWrapper::get_region(GroupType t)
{
    return VkStridedDeviceAddressRegionKHR(get_address(t),get_stride(t),get_size(t));
}

const std::array<VkStridedDeviceAddressRegionKHR, 4> SBTWrapper::get_regions()
{
    std::array<VkStridedDeviceAddressRegionKHR, 4> regions
    {
        get_region(RayGen), get_region(Miss), get_region(Hit),get_region(Callable)
    };

    return regions;
}
