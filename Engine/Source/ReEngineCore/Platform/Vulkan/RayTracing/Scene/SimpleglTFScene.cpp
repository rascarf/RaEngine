#include "SimpleglTFScene.h"

#define TINYGLTF_IMPLEMENTATION
// #define TINYGLTF_NO_STB_IMAGE_WRITE
#include "cgltf/include/tiny_gltf.h"

#include "glm/detail/type_quat.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "Math/Math.h"
#include "Resource/AssetManager/AssetManager.h"

    void glTFScene::LoadglTFModel(Ref<VulkanDevice> device,Ref<VulkanCommandBuffer> cmdBuffer, std::string FilePath, bool bIsBinary)
{
    CmdBuffer = cmdBuffer;
    Device = device;
    
    tinygltf::Model gltfModel;
    tinygltf::TinyGLTF gltfContext;
    std::string error;
    std::string warning;

    if(bIsBinary)
    {
        uint32 dataSize = 0;
        uint8* dataPtr  = nullptr;
        AssetManager::ReadFile(FilePath, dataPtr, dataSize);
        gltfContext.LoadBinaryFromMemory(&gltfModel, &error, &warning, dataPtr, dataSize);
    }
    else
    {
        gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning,FilePath);
    }
    
    LoadTextures(cmdBuffer, gltfModel);
    LoadMaterials(cmdBuffer, gltfModel);
    LoadMeshes(cmdBuffer, gltfModel);
    LoadNodes(cmdBuffer, gltfModel);
    LoadLights(cmdBuffer, gltfModel);
}

void glTFScene::LoadTextures(Ref<VulkanCommandBuffer> cmdBuffer, tinygltf::Model& gltfModel)
{
    for (int32 i = 0; i < gltfModel.textures.size(); ++i)
    {
        tinygltf::Texture& tex = gltfModel.textures[i];
        tinygltf::Image& image = gltfModel.images[tex.source];
        Ref<VulkanTexture> texture = VulkanTexture::Create2D(image.image.data(), image.width * image.height * 4, VK_FORMAT_R8G8B8A8_UNORM, image.width, image.height, Device, cmdBuffer);


        Textures.push_back(texture);
    }
}

void glTFScene::LoadMaterials(Ref<VulkanCommandBuffer> cmdBuffer, tinygltf::Model& gltfModel)
{
    for (int32 i = 0; i < gltfModel.materials.size(); ++i)
    {
        tinygltf::Material& gltfMat = gltfModel.materials[i];
        Material material;

        if (gltfMat.values.find("baseColorTexture") != gltfMat.values.end())
        {
            material.TextureID.x = gltfMat.values["baseColorTexture"].TextureIndex();
        }

        if (gltfMat.values.find("roughnessFactor") != gltfMat.values.end())
        {
            material.Params.x = gltfMat.values["roughnessFactor"].Factor();
        }

        if (gltfMat.values.find("metallicFactor") != gltfMat.values.end())
        {
            material.Params.y = gltfMat.values["metallicFactor"].Factor();
        }

        if (gltfMat.values.find("baseColorFactor") != gltfMat.values.end())
        {
            material.Albedo.x = gltfMat.values["baseColorFactor"].ColorFactor().data()[0];
            material.Albedo.y = gltfMat.values["baseColorFactor"].ColorFactor().data()[1];
            material.Albedo.z = gltfMat.values["baseColorFactor"].ColorFactor().data()[2];
            material.Albedo.w = gltfMat.values["baseColorFactor"].ColorFactor().data()[3];
        }

        Materials.push_back(material);
    }
}

void glTFScene::LoadMeshes(Ref<VulkanCommandBuffer> cmdBuffer, tinygltf::Model& gltfModel)
{for (int32 i = 0; i < gltfModel.meshes.size(); ++i)
        {
            tinygltf::Mesh& gltfMesh = gltfModel.meshes[i];

            for (int32 j = 0; j < gltfMesh.primitives.size(); ++j)
            {
                tinygltf::Primitive& primitive = gltfMesh.primitives[j];

                std::vector<float> vertices;
                std::vector<uint32> indices;
                BoundingBox bounds;

                // vertices
                uint8* bufferPos = nullptr;
                uint8* bufferNormals = nullptr;
                uint8* bufferUV0 = nullptr;
                uint8* bufferTangents = nullptr;

                tinygltf::Accessor& posAccessor = gltfModel.accessors[primitive.attributes.find("POSITION")->second];
                tinygltf::BufferView& posView = gltfModel.bufferViews[posAccessor.bufferView];
                bufferPos = &(gltfModel.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]);

                if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
                {
                    tinygltf::Accessor& normAccessor = gltfModel.accessors[primitive.attributes.find("NORMAL")->second];
                    tinygltf::BufferView& normView = gltfModel.bufferViews[normAccessor.bufferView];
                    bufferNormals = &(gltfModel.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]);
                }

                if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
                {
                    tinygltf::Accessor& uvAccessor = gltfModel.accessors[primitive.attributes.find("TEXCOORD_0")->second];
                    tinygltf::BufferView& uvView = gltfModel.bufferViews[uvAccessor.bufferView];
                    bufferUV0 = &(gltfModel.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]);
                }

                if (primitive.attributes.find("TANGENT") != primitive.attributes.end())
                {
                    tinygltf::Accessor& tangentAccessor = gltfModel.accessors[primitive.attributes.find("TANGENT")->second];
                    tinygltf::BufferView& tangentView = gltfModel.bufferViews[tangentAccessor.bufferView];
                    bufferTangents = &(gltfModel.buffers[tangentView.buffer].data[tangentAccessor.byteOffset + tangentView.byteOffset]);
                }

                for (int32 v = 0; v < posAccessor.count; ++v)
                {
                    // pos
                    {
                        const float* buf = (const float*)(bufferPos);
                        float px = buf[v * 3 + 0];
                        float py = buf[v * 3 + 1];
                        float pz = buf[v * 3 + 2];

                        if (v == 0)
                        {
                            bounds.Min.x = bounds.Max.x = px;
                            bounds.Min.y = bounds.Max.y = py;
                            bounds.Min.z = bounds.Max.z = pz;
                        }

                        bounds.Min.x = Math::Min(bounds.Min.x, px);
                        bounds.Min.y = Math::Min(bounds.Min.y, py);
                        bounds.Min.z = Math::Min(bounds.Min.z, pz);
                        bounds.Max.x = Math::Max(bounds.Max.x, px);
                        bounds.Max.y = Math::Max(bounds.Max.y, py);
                        bounds.Max.z = Math::Max(bounds.Max.z, pz);

                        vertices.push_back(px);
                        vertices.push_back(py);
                        vertices.push_back(pz);
                    }
                    // uv
                    {
                        if (bufferUV0)
                        {
                            const float* buf = (const float*)(bufferUV0);
                            vertices.push_back(buf[v * 2 + 0]);
                            vertices.push_back(buf[v * 2 + 1]);
                        }
                        else
                        {
                            vertices.push_back(0);
                            vertices.push_back(0);
                        }
                    }
                    // normal
                    {
                        if (bufferNormals)
                        {
                            const float* buf = (const float*)(bufferNormals);
                            vertices.push_back(buf[v * 3 + 0]);
                            vertices.push_back(buf[v * 3 + 1]);
                            vertices.push_back(buf[v * 3 + 2]);
                        }
                        else
                        {
                            vertices.push_back(0);
                            vertices.push_back(0);
                            vertices.push_back(0);
                        }
                    }
                    // tangent
                    {
                        if (bufferTangents)
                        {
                            const float* buf = (const float*)(bufferTangents);
                            vertices.push_back(buf[v * 3 + 0]);
                            vertices.push_back(buf[v * 3 + 1]);
                            vertices.push_back(buf[v * 3 + 2]);
                        }
                        else
                        {
                            vertices.push_back(0);
                            vertices.push_back(0);
                            vertices.push_back(0);
                        }
                    }
                }

                // indices
                tinygltf::Accessor& indicesAccessor = gltfModel.accessors[primitive.indices];
                tinygltf::BufferView& indicesBufferView = gltfModel.bufferViews[indicesAccessor.bufferView];
                uint8* bufferIndices = &(gltfModel.buffers[indicesBufferView.buffer].data[indicesAccessor.byteOffset + indicesBufferView.byteOffset]);

                for (int32 v = 0; v < indicesAccessor.count; ++v)
                {
                    if (indicesAccessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT)
                    {
                        const uint32* buf = (const uint32*)(bufferIndices);
                        indices.push_back(buf[v]);
                    }
                    else if (indicesAccessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT)
                    {
                        const uint16* buf = (const uint16*)(bufferIndices);
                        indices.push_back(buf[v]);
                    }
                    else if (indicesAccessor.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT)
                    {
                        const uint8* buf = (const uint8*)(bufferIndices);
                        indices.push_back(buf[v]);
                    }
                }

                glTFMesh* mesh = new glTFMesh();
                mesh->Bounds = bounds;
                mesh->VertexStride = 11 * sizeof(float);
                mesh->VertexCount  = vertices.size();
                {
                    Ref<VulkanBuffer> vertexStaging = VulkanBuffer::CreateBuffer(
                        Device,
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        vertices.size() * sizeof(float),
                        vertices.data()
                    );

                    mesh->VertexBuffer = VulkanBuffer::CreateBuffer(
                        Device,
                        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        vertices.size() * sizeof(float)
                    );

                    cmdBuffer->Begin();

                    VkBufferCopy copyRegion = {};
                    copyRegion.size = vertices.size() * sizeof(float);
                    vkCmdCopyBuffer(cmdBuffer->CmdBuffer, vertexStaging->Buffer, mesh->VertexBuffer->Buffer, 1, &copyRegion);

                    cmdBuffer->End();
                    cmdBuffer->Submit();
                    
                }

                mesh->IndexCount = indices.size();
                {
                   Ref<VulkanBuffer> indexStaging = VulkanBuffer::CreateBuffer(
                        Device,
                        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                        indices.size() * sizeof(uint32),
                        indices.data()
                    );

                    mesh->IndexBuffer = VulkanBuffer::CreateBuffer(
                        Device,
                        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        indices.size() * sizeof(uint32)
                    );

                    cmdBuffer->Begin();

                    VkBufferCopy copyRegion = {};
                    copyRegion.size = indices.size() * sizeof(uint32);

                    vkCmdCopyBuffer(cmdBuffer->CmdBuffer, indexStaging->Buffer, mesh->IndexBuffer->Buffer, 1, &copyRegion);

                    cmdBuffer->End();
                    cmdBuffer->Submit();
                    
                }

                mesh->Material = primitive.material;
                Meshes.push_back(mesh);
            }
        }
}

void glTFScene::LoadNodes(Ref<VulkanCommandBuffer> cmdBuffer, tinygltf::Model& gltfModel)
{
    tinygltf::Scene &scene = gltfModel.scenes[0];

    RootNode = new Node();

    for (int32 i = 0; i < scene.nodes.size(); ++i)
    {
        tinygltf::Node& gltfNode = gltfModel.nodes[scene.nodes[i]];
        LoadNode(RootNode, gltfNode, gltfModel);
    }
}

void glTFScene::LoadNode(Node* parent, tinygltf::Node& gltfNode, tinygltf::Model &gltfModel)
{
    Node* node = new Node();
    node->Name = gltfNode.name;
    
    node->Transform = glm::identity<glm::mat4>();

    
    Nodes.push_back(node);

    if (gltfNode.rotation.size() == 4)
    {
        glm::quat qua(gltfNode.rotation[3],gltfNode.rotation[0], gltfNode.rotation[1], gltfNode.rotation[2]);
        node->Transform = node->Transform * glm::toMat4(qua);
    }

    if (gltfNode.scale.size() == 3)
    {
       node->Transform =  glm::scale(node->Transform,glm::vec3(gltfNode.scale[0], gltfNode.scale[1], gltfNode.scale[2]));
    }

    if (gltfNode.translation.size() == 3)
    {
        node->Transform = glm::translate(node->Transform,glm::vec3(gltfNode.translation[0], gltfNode.translation[1], gltfNode.translation[2]));
    }

    if (gltfNode.matrix.size() == 16)
    {
        node->Transform[0][0] = (float)gltfNode.matrix[0];
        node->Transform[0][1] = (float)gltfNode.matrix[1];
        node->Transform[0][2] = (float)gltfNode.matrix[2];
        node->Transform[0][3] = (float)gltfNode.matrix[3];

        node->Transform[1][0] = (float)gltfNode.matrix[4];
        node->Transform[1][1] = (float)gltfNode.matrix[5];
        node->Transform[1][2] = (float)gltfNode.matrix[6];
        node->Transform[1][3] = (float)gltfNode.matrix[7];

        node->Transform[2][0] = (float)gltfNode.matrix[8];
        node->Transform[2][1] = (float)gltfNode.matrix[9];
        node->Transform[2][2] = (float)gltfNode.matrix[10];
        node->Transform[2][3] = (float)gltfNode.matrix[11];

        node->Transform[3][0] = (float)gltfNode.matrix[12];
        node->Transform[3][1] = (float)gltfNode.matrix[13];
        node->Transform[3][2] = (float)gltfNode.matrix[14];
        node->Transform[3][3] = (float)gltfNode.matrix[15];
    }

    node->Parent = parent;
    if (parent)
    {
        parent->Children.push_back(node);
    }

    if (gltfNode.mesh > -1)
    {
        node->MeshIndex = gltfNode.mesh;

        Entities.push_back(node);
    }

    for (int32 i = 0; i < gltfNode.children.size(); ++i)
    {
        LoadNode(node, gltfModel.nodes[gltfNode.children[i]], gltfModel);
    }
}

void glTFScene::LoadLights(Ref<VulkanCommandBuffer> cmdBuffer, tinygltf::Model& gltfModel)
{
    // default sky light
    Lights.push_back(new SkyEnvLight(glm::vec3(0.5f, 0.5f, 1.0f)));

    for (int32 i = 0; i < gltfModel.lights.size(); ++i)
    {
        tinygltf::Light& light = gltfModel.lights[i];
    }

    if (gltfModel.lights.size() == 0)
    {
        DirectionalLight* defaultLight = new DirectionalLight(normalize(glm::vec3(1, -1, 1)), glm::vec3(5, 5, 5));
        Lights.push_back(defaultLight);
    }

    UpdateLightData();
}

void glTFScene::CreateBlas()
{
    std::vector<BlasInput> BlasInputs;

    for(auto& PrimMesh : Meshes)
    {
        BlasInput Geo = BlasInput::MeshToVkGeometry(*PrimMesh,PrimMesh->VertexBuffer,PrimMesh->IndexBuffer);
        BlasInputs.push_back({Geo});
    }

    BuildBlas(BlasInputs,VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
}

// Create all the BLAS from the vector of BlasInput
// - There will be one BLAS per input-vector entry
// - There will be as many BLAS as input.size()
// - The resulting BLAS (along with the inputs used to build) are stored inm_blas and can be referenced by index.
// - if flag has the 'Compact' flag, the BLAS will be compacted
void glTFScene::BuildBlas(const std::vector<BlasInput>& input, VkBuildAccelerationStructureFlagsKHR flags)
{
    uint32_t NumberBlas = static_cast<uint32_t>(input.size());

    VkDeviceSize ASTotalSize {0};
    uint32_t NumberCompaction{0};
    VkDeviceSize MaxScratchSize{0};

    std::vector<BuildAccelerationStructure> BuildAs(NumberBlas);
    for(uint32_t Index = 0 ; Index < NumberBlas ; Index++)
    {
        BuildAs[Index].build_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        BuildAs[Index].build_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        BuildAs[Index].build_info.flags = input[Index].Flags | flags;
        BuildAs[Index].build_info.geometryCount = static_cast<uint32_t>(input[Index].AsGeom.size());
        BuildAs[Index].build_info.pGeometries = input[Index].AsGeom.data();

        BuildAs[Index].range_info = input[Index].ASBuildOffsetInfo.data();

        // Finding sizes to create acceleration structures and scratch
        std::vector<uint32_t> MaxPrimCount(input[Index].ASBuildOffsetInfo.size());
        for(uint32_t tt = 0 ; tt < input[Index].ASBuildOffsetInfo.size() ; tt++)
        {
            MaxPrimCount[tt] = input[Index].ASBuildOffsetInfo[tt].primitiveCount;
        }
        
        vkGetAccelerationStructureBuildSizesKHR(
            Device->GetInstanceHandle(),
            VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
            &BuildAs[Index].build_info,
            MaxPrimCount.data(),
            &BuildAs[Index].size_info
        );

        // Extra info
        ASTotalSize += BuildAs[Index].size_info.accelerationStructureSize;
        MaxScratchSize = std::max<VkDeviceSize>(MaxScratchSize,BuildAs[Index].size_info.buildScratchSize);

        // VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR is needed！
        NumberCompaction += BuildAs[Index].build_info.flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR == VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR ;
    }

    // Scratch Buffer holding tempraray data of the acceleration structure buffer
    Ref<VulkanBuffer> ScratchBuffer = VulkanBuffer::CreateBuffer(
        Device,
        VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        MaxScratchSize
    );

    VkQueryPool QueryPool{VK_NULL_HANDLE};
    if(NumberCompaction)
    {
        VkQueryPoolCreateInfo gPci{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
        gPci.queryCount = NumberBlas;
        gPci.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR;
        vkCreateQueryPool(Device->GetInstanceHandle(),&gPci,nullptr,&QueryPool);
    }

    // Batching creation/compaction of BLAS to allow staying in restricted amount of memory
    std::vector<uint32_t> Indices; // Blas to Create
    VkDeviceSize BatchSize{0};
    VkDeviceSize BatchLimit{256'000'000};// 256MB

    for(uint32_t Index = 0 ; Index < NumberBlas ; Index++)
    {
        Indices.push_back(Index);
        BatchSize += BuildAs[Index].size_info.accelerationStructureSize;

        // if Over the limit or last blas element

        if(BatchSize >= BatchLimit || Index == NumberBlas - 1)
        {
            CmdBuffer->Begin();
            CmdCreateBlas(Indices,BuildAs,ScratchBuffer->GetDeviceAddress(),QueryPool);
            CmdBuffer->End();
            CmdBuffer->Submit();
            
            if(QueryPool)
            {
                CmdCreateCompact(Indices,BuildAs,QueryPool);
                CmdBuffer->Submit();

                //destory non-compacted version
                for(auto i : Indices)
                {
                    vkDestroyAccelerationStructureKHR(Device->GetInstanceHandle(), BuildAs[i].Cleanup_as.accel, nullptr);
                    BuildAs[i].Cleanup_as.buffer.reset();
                }
            }

            BatchSize = 0;
            Indices.clear();
        }
    }

    // Logging reduction
    if (QueryPool)
    {
        VkDeviceSize compact_size = std::accumulate(BuildAs.begin(), BuildAs.end(), 0ULL,[](const auto& a, const auto& b){ return a + b.size_info.accelerationStructureSize; });
        RE_CORE_INFO(" RT BLAS: reducing from: {0} to: {1} = {2} ({3}{4} smaller) \n", ASTotalSize, compact_size,ASTotalSize - compact_size, (ASTotalSize - compact_size) / float(ASTotalSize) * 100.f, "%");
    }

    for(auto& b : BuildAs)
    {
        Blases.emplace_back(b.As);
    }

    // Clean up
    vkDestroyQueryPool(Device->GetInstanceHandle(),QueryPool,nullptr);
    ScratchBuffer.reset();
}

void glTFScene::CmdCreateBlas(std::vector<uint32_t> indices, std::vector<BuildAccelerationStructure>& buildAs,VkDeviceAddress scratchAddress, VkQueryPool queryPool)
{
    if(queryPool)
    {
        vkResetQueryPool(Device->GetInstanceHandle(),queryPool,0,static_cast<uint32_t>(indices.size()));
    }

    uint32_t QueryCount{0};
    for(const auto& Index : indices)
    {
        VkAccelerationStructureCreateInfoKHR CreateInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
        CreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        CreateInfo.size = buildAs[Index].size_info.accelerationStructureSize;
        buildAs[Index].As = AccelKHR::CreateAcceleration(Device,CreateInfo);

        //#2
        buildAs[Index].build_info.dstAccelerationStructure = buildAs[Index].As.accel;
        buildAs[Index].build_info.scratchData.deviceAddress = scratchAddress;
        vkCmdBuildAccelerationStructuresKHR(CmdBuffer->CmdBuffer,1,&buildAs[Index].build_info,&buildAs[Index].range_info);

        VkMemoryBarrier barrier{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
        barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
        barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        vkCmdPipelineBarrier(CmdBuffer->CmdBuffer,
            VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                             VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                             0,
                             1,
                             &barrier,
                             0,
                             nullptr,
                             0,
                             nullptr);

        if(queryPool)
        {
            vkCmdWriteAccelerationStructuresPropertiesKHR(
                CmdBuffer->CmdBuffer,
                1,
                &buildAs[Index].build_info.dstAccelerationStructure,
                VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR,
                queryPool,
                QueryCount++
            );
        }
    }
}

void glTFScene::CmdCreateCompact(std::vector<uint32_t> indices, std::vector<BuildAccelerationStructure>& buildAs,VkQueryPool queryPool)
{
    uint32_t query_cnt{0};
    std::vector<AccelKHR> cleanupAS;  // previous AS to destroy

    // Get the compacted size result back
    std::vector<VkDeviceSize> compact_sizes(static_cast<uint32_t>(indices.size()));
    vkGetQueryPoolResults(Device->GetInstanceHandle(), queryPool, 0, (uint32_t)compact_sizes.size(),
                          compact_sizes.size() * sizeof(VkDeviceSize), compact_sizes.data(), sizeof(VkDeviceSize),
                          VK_QUERY_RESULT_WAIT_BIT);

    for(auto Index : indices)
    {
        buildAs[Index].Cleanup_as = buildAs[Index].As;
        buildAs[Index].size_info.accelerationStructureSize = compact_sizes[query_cnt];
        VkAccelerationStructureCreateInfoKHR asCreateInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
        asCreateInfo.size = buildAs[Index].size_info.accelerationStructureSize;
        asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        buildAs[Index].As = AccelKHR::CreateAcceleration(Device,asCreateInfo);

        VkCopyAccelerationStructureInfoKHR copyInfo{VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR};
        copyInfo.src = buildAs[Index].build_info.dstAccelerationStructure;
        copyInfo.dst = buildAs[Index].As.accel;
        copyInfo.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR;
        vkCmdCopyAccelerationStructureKHR(CmdBuffer->CmdBuffer, &copyInfo);
    }
}


