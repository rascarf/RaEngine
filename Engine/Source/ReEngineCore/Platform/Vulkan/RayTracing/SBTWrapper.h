#pragma once
#include <array>

#include "Platform/Vulkan/VulkanCommonDefine.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanBuffer.h"

namespace ReEngine
{
    class SBTWrapper
    {
    public:
        enum GroupType
        {
            RayGen,
            Miss,
            Hit,
            Callable
        };

        void Setup(VulkanContext* ctx,uint32 FamilyIndex, const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& RtProps);
        void Destroy();

        void Create(
            VkPipeline RtPipeline,
            VkRayTracingPipelineCreateInfoKHR Pipeline_Info,
            const std::vector<VkRayTracingPipelineCreateInfoKHR> & CreateInfos = {} //
        );

        void AddIndices(VkRayTracingPipelineCreateInfoKHR pipeline_info,
                     const std::vector<VkRayTracingPipelineCreateInfoKHR>& create_infos = {});

        void AddIndex(GroupType t,uint32_t Index)
        {
            mIndex[t].push_back(Index);
        }
        
        template<typename T>
        void AddData(GroupType t,uint32_t GroupIndex,T& Data)
        {
            AddDataInternal(t, GroupIndex,(uint8*)&Data,sizeof(T));
        }

        void AddDataInternal(GroupType t,uint32_t GroupIndex,uint8* Data,size_t DataSize)
        {
            std::vector<uint8_t> Dst(Data,Data + DataSize);
            mData[t][GroupIndex] = Dst;
        }
        
        VulkanContext*                      mCtx;

        uint32_t index_count(GroupType t) { return static_cast<uint32_t>(mIndex[t].size()); }
        uint32_t get_stride(GroupType t) { return mStride[t]; }
        uint32_t get_size(GroupType t) { return get_stride(t) * index_count(t); }
        VkDeviceAddress get_address(GroupType t);
        const VkStridedDeviceAddressRegionKHR get_region(GroupType t);
        const std::array<VkStridedDeviceAddressRegionKHR, 4> get_regions();
        
    private:
        using entry = std::unordered_map<uint32_t, std::vector<uint8_t>>;
        std::array<std::vector<uint32_t>,4> mIndex; // 速查表，各个ShaderRecord从哪里开始
        std::array<Ref<VulkanBuffer>,4>     mBuffer;// 各个ShaderRecord的Buffer
        std::array<uint32_t,4>              mStride{0,0,0,0};// 各个ShaderRecord的Stride
        std::array<entry,4>                 mData; //各个ShaderRecord的数据，Map的形式是为了快速查

        uint32_t                            HandleSize;
        uint32_t                            HandleAlignment;
        uint32_t                            QueueIndex;

    };
}

