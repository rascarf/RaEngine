#pragma once
#include "Core/Core.h"
#include "glm/fwd.hpp"
#include "glm/matrix.hpp"
#include "Platform/Vulkan/VulkanCommonDefine.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanBuffer.h"

namespace ReEngine
{
    class glTFMesh;
    
    // Geometry--- BlasInput
    // 出于拓展性考虑，这里用数组
    struct BlasInput
    {
        std::vector<VkAccelerationStructureGeometryKHR> AsGeom; // 一般来说都只有一个
        std::vector<VkAccelerationStructureBuildRangeInfoKHR> ASBuildOffsetInfo; // 一般来说都只有一个
        VkBuildAccelerationStructureFlagsKHR Flags{0};
    
        static BlasInput MeshToVkGeometry(glTFMesh& Prim, Ref<VulkanBuffer> VertexBuffer, Ref<VulkanBuffer> IndexBuffer);
    };
    
    // 生成的AccelerationStruct
    struct AccelKHR
    {
        VkAccelerationStructureKHR accel = VK_NULL_HANDLE;
        Ref<VulkanBuffer> buffer;
    
        static AccelKHR CreateAcceleration(Ref<VulkanDevice> Device,VkAccelerationStructureCreateInfoKHR& accel)
        {
            AccelKHR ResultAccel;
    
            Ref<VulkanBuffer> AccelBuffer = VulkanBuffer::CreateBuffer(
                Device,
                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                accel.size
            );
    
            ResultAccel.buffer = AccelBuffer;
            accel.buffer = AccelBuffer->Buffer;
    
            vkCreateAccelerationStructureKHR(Device->GetInstanceHandle(),&accel,nullptr,&ResultAccel.accel);
            return ResultAccel;
        }

        static VkTransformMatrixKHR ToVkMatrix(const glm::mat4 mat)
        {
            // TODO 得验证一下行列顺序
            glm::mat4 Temp = glm::transpose(mat);
            VkTransformMatrixKHR out_matrix;
            memcpy(&out_matrix, &Temp, sizeof(VkTransformMatrixKHR));
            return out_matrix;
        }

        VkDeviceAddress GetDeviceAddress(VkDevice device)
        {
            VkAccelerationStructureDeviceAddressInfoKHR AddrInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR};
            AddrInfo.accelerationStructure = accel;
            return vkGetAccelerationStructureDeviceAddressKHR(device,&AddrInfo);
        }
    };
    
    // 创建AccelerationStruct的结构体
    struct BuildAccelerationStructure
    {
        VkAccelerationStructureBuildGeometryInfoKHR build_info{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
        VkAccelerationStructureBuildSizesInfoKHR size_info{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
        const VkAccelerationStructureBuildRangeInfoKHR* range_info;
    
        AccelKHR As;  // result acceleration structure
        AccelKHR Cleanup_as; // Compact version use this field
    };
}


