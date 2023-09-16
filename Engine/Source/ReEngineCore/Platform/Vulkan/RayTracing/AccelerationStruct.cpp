#include "AccelerationStruct.h"
#include "Scene/SimpleglTFScene.h"

namespace ReEngine
{
    BlasInput BlasInput::MeshToVkGeometry(glTFMesh& Prim, Ref<VulkanBuffer> VertexBuffer, Ref<VulkanBuffer> IndexBuffer)
    {
        uint32_t MaxPrimitiveCount = Prim.IndexCount / 3;
    
        // 也可以使用 VkAccelerationStructureGeometryDataKHR的 triangle字段
        VkAccelerationStructureGeometryTrianglesDataKHR Triangles
        {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR};
    
        Triangles.vertexFormat = Prim.VertexPositionFormat;
        Triangles.vertexStride = Prim.VertexStride;
        Triangles.vertexData.deviceAddress = VertexBuffer->GetDeviceAddress();
    
        // 32-bit uint
        Triangles.indexType = Prim.MeshIndexFormat;
        Triangles.indexData.deviceAddress = IndexBuffer->GetDeviceAddress();
    
        Triangles.maxVertex = Prim.VertexCount;
    
        VkAccelerationStructureGeometryKHR AsGeom{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
        AsGeom.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        AsGeom.flags = VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR; // For AnyHit
        AsGeom.geometry.triangles = Triangles;
    
        VkAccelerationStructureBuildRangeInfoKHR Offset;
        Offset.primitiveCount = MaxPrimitiveCount;
        Offset.firstVertex = 0;
        Offset.primitiveOffset = 0;
        Offset.transformOffset = 0;
    
        BlasInput Input;
        Input.AsGeom.emplace_back(AsGeom);
        Input.ASBuildOffsetInfo.emplace_back(Offset);
    
        return Input;
    }
}

