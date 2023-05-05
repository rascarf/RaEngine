#include "VulkanMesh.h"

#include "assimp/config.h"
#include <assimp/Importer.hpp>
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/code/glTF2/glTF2Importer.h"
#include "glm/ext/matrix_transform.hpp"
#include "Resource/AssetManager/AssetManager.h"

using namespace std;
VkVertexInputBindingDescription VulkanModel::GetInputBinding()
{
    int32 stride = 0;
    for (int32 i = 0; i < Attributes.size(); ++i)
    {
        stride += VertexAttributeToSize(Attributes[i]);
    }  

    VkVertexInputBindingDescription vertexInputBinding = {};
    vertexInputBinding.binding   = 0;
    vertexInputBinding.stride    = stride;
    vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return vertexInputBinding;
}

std::vector<VkVertexInputAttributeDescription> VulkanModel::GetInputAttributes()
{
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributs;
    int32 offset = 0;

    for (int32 i = 0; i < Attributes.size(); ++i)
    {
        VkVertexInputAttributeDescription inputAttribute = {};
        inputAttribute.binding  = 0;
        inputAttribute.location = i;
        inputAttribute.format   = VertexAttributeToVkFormat(Attributes[i]);
        inputAttribute.offset   = offset;
        offset += VertexAttributeToSize(Attributes[i]);
        vertexInputAttributs.push_back(inputAttribute);
    }

    return vertexInputAttributs;
}

Ref<VulkanModel> VulkanModel::LoadFromFile(const std::string& filename, Ref<VulkanDevice> vulkanDevice,Ref<VulkanCommandBuffer> cmdBuffer, const std::vector<VertexAttribute>& attributes)
{
    Ref<VulkanModel> model   = CreateRef<VulkanModel>();
    model->Device     = vulkanDevice;
    model->Attributes = attributes;
    model->CmdBuffer  = cmdBuffer;
        
    int assimpFlags =
        aiProcess_Triangulate | //三角形
        aiProcess_MakeLeftHanded | //左手系坐标
        aiProcess_FlipUVs | // 反转UV的Y坐标
        aiProcess_FlipWindingOrder; //翻转绕序
        
    for (int32 i = 0; i < attributes.size(); ++i)
    {
        if (attributes[i] == VertexAttribute::VA_Tangent)
        {
            assimpFlags = assimpFlags | aiProcess_CalcTangentSpace;
        }
        else if (attributes[i] == VertexAttribute::VA_UV0)
        {
            assimpFlags = assimpFlags | aiProcess_GenUVCoords;
        }
        else if (attributes[i] == VertexAttribute::VA_Normal)
        {
            assimpFlags = assimpFlags | aiProcess_GenSmoothNormals;
        }
    }

    uint32 dataSize = 0;
    uint8* dataPtr  = nullptr;
    if (!AssetManager::ReadFile(filename, dataPtr, dataSize))
    {
        RE_CORE_ERROR("Can't Load File");
        return model;
    }

    using namespace Assimp;
    Assimp::Importer importer;
    auto scene = importer.ReadFileFromMemory(dataPtr, dataSize, assimpFlags);
    
    model->LoadNode(scene->mRootNode, scene);

    delete[] dataPtr;

    return model;
}

Ref<VulkanModel> VulkanModel::Create(std::shared_ptr<VulkanDevice> vulkanDevice, Ref<VulkanCommandBuffer> cmdBuffer,const std::vector<float>& vertices, const std::vector<uint16>& indices,const std::vector<VertexAttribute>& attributes)
{
    Ref<VulkanModel> model   = CreateRef<VulkanModel>();
    model->Device     = vulkanDevice;
    model->Attributes = attributes;
    model->CmdBuffer  = cmdBuffer;

    int32 stride = 0;
    for (int32 i = 0; i < attributes.size(); ++i)
    {
        stride += VertexAttributeToSize(attributes[i]);
    }

    Ref<VulkanPrimitive> primitive = CreateRef<VulkanPrimitive>();
    primitive->vertices     = vertices;
    primitive->indices      = indices;
    primitive->vertexCount  = (int32)vertices.size() / stride * 4;

    if (cmdBuffer)
    {
        if (vertices.size() > 0)
        {
            primitive->VertexBuffer = VulkanVertexBuffer::Create(vulkanDevice, cmdBuffer, primitive->vertices, attributes);
        }
        if (indices.size() > 0)
        {
            primitive->IndexBuffer = VulkanIndexBuffer::Create(vulkanDevice, cmdBuffer, primitive->indices);
        }
    }

    Ref<VulkanMesh> mesh = CreateRef<VulkanMesh>();
    mesh->m_Primitives.push_back(primitive);
    mesh->m_BoundingBox.Min = glm::vec3(-1.0f, -1.0f, 0.0f);
    mesh->m_BoundingBox.Max = glm::vec3(1.0f, 1.0f, 0.0f);
    
    Ref<VulkanMeshNode> rootNode = CreateRef<VulkanMeshNode>();
    rootNode->name = "RootNode";
    rootNode->Meshes.push_back(mesh);
    rootNode->LocalMatrix = glm::identity<glm::mat4>();
    mesh->LinkNode = rootNode;

    model->RootNode = rootNode;
    model->Meshes.push_back(mesh);

    return model;
}

Ref<VulkanMeshNode> VulkanModel::LoadNode(const aiNode* Innode, const aiScene* Inscene)
{
    Ref<VulkanMeshNode> Node = CreateRef<VulkanMeshNode>();
    Node->name = Innode->mName.C_Str();

    if(RootNode == nullptr)
    {
        RootNode = Node;
    }

    FillMatrixWithAiMatrix(Node->LocalMatrix,Innode->mTransformation);
    
    // mesh
    if(Innode->mNumMeshes > 0)
    {
        for(uint32 i = 0 ; i < Innode->mNumMeshes ; i++)
        {
            Ref<VulkanMesh> Mesh = LoadMesh(Inscene->mMeshes[Innode->mMeshes[i]],Inscene);
            Mesh->LinkNode = Node;
            Node->Meshes.push_back(Mesh);
            Meshes.push_back(Mesh);
        }
    }

    LinearNodes.push_back(Node);
    
    // children node
    for (int32 i = 0; i < (int32)Innode->mNumChildren; ++i)
    {
        Ref<VulkanMeshNode> childNode = LoadNode(Innode->mChildren[i], Inscene);
        childNode->Parent  = std::weak_ptr(Node);
        Node->Children.push_back(childNode);
    }

    return Node;
}

Ref<VulkanMesh> VulkanModel::LoadMesh(const aiMesh* Inmesh, const aiScene* Inscene)
{
    Ref<VulkanMesh> Mesh = CreateRef<VulkanMesh>();

    // // load material
    // aiMaterial* material = aiScene->mMaterials[aiMesh->mMaterialIndex];
    // if (material)
    // {
    //     FillMaterialTextures(material, mesh->material);
    // }

    // // load bones
    // std::unordered_map<uint32, DVKVertexSkin> skinInfoMap;
    // if (aiMesh->mNumBones > 0 && loadSkin)
    // {
    //     LoadSkin(skinInfoMap, mesh, aiMesh, aiScene);
    // }

    // load vertex data
    std::vector<float> vertices;
    glm::vec3 mmin( MAX_FLT,  MAX_FLT,  MAX_FLT);
    glm::vec3 mmax(-MAX_FLT, -MAX_FLT, -MAX_FLT);
    LoadVertexDatas(vertices, mmax, mmin, Mesh, Inmesh, Inscene);

    // load indices
    std::vector<uint32> indices;
    LoadIndices(indices, Inmesh, Inscene);

    // load primitives
    LoadPrimitives(vertices, indices, Mesh, Inmesh, Inscene);

    Mesh->m_BoundingBox.Min = mmin;
    Mesh->m_BoundingBox.Max = mmax;
    Mesh->m_BoundingBox.UpdateCorners();
    
    return Mesh;
}

void VulkanModel::FillMatrixWithAiMatrix(glm::mat4x4& OutMatix, const aiMatrix4x4& aiMatrix)
{
    OutMatix[0][0] = aiMatrix.a1;
    OutMatix[0][1] = aiMatrix.a2;
    OutMatix[0][2] = aiMatrix.a3;
    OutMatix[0][3] = aiMatrix.a4;
    OutMatix[1][0] = aiMatrix.b1;
    OutMatix[1][1] = aiMatrix.b2;
    OutMatix[1][2] = aiMatrix.b3;
    OutMatix[1][3] = aiMatrix.b4;
    OutMatix[2][0] = aiMatrix.c1;
    OutMatix[2][1] = aiMatrix.c2;
    OutMatix[2][2] = aiMatrix.c3;
    OutMatix[2][3] = aiMatrix.c4;
    OutMatix[3][0] = aiMatrix.d1;
    OutMatix[3][1] = aiMatrix.d2;
    OutMatix[3][2] = aiMatrix.d3;
    OutMatix[3][3] = aiMatrix.d4;

    OutMatix = glm::transpose(OutMatix);
}

void VulkanModel::LoadVertexDatas(std::vector<float>& vertices, glm::vec3& mmax, glm::vec3& mmin, Ref<VulkanMesh> mesh,const aiMesh* aiMesh, const aiScene* aiScene)
{
    glm::vec3 defaultColor(0.5,0.5,0.5);

    for (int32 i = 0; i < (int32)aiMesh->mNumVertices; ++i)
        {
            for (int32 j = 0; j < Attributes.size(); ++j)
            {
                if (Attributes[j] == VertexAttribute::VA_Position)
                {
                    float v0 = aiMesh->mVertices[i].x;
                    float v1 = aiMesh->mVertices[i].y;
                    float v2 = aiMesh->mVertices[i].z;

                    vertices.push_back(v0);
                    vertices.push_back(v1);
                    vertices.push_back(v2);

                    mmin.x = min(v0, mmin.x);
                    mmin.y = min(v1, mmin.y);
                    mmin.z = min(v2, mmin.z);
                    mmax.x = max(v0, mmax.x);
                    mmax.y = max(v1, mmax.y);
                    mmax.z = max(v2, mmax.z);
                }
                else if (Attributes[j] == VertexAttribute::VA_UV0)
                {
                    if (aiMesh->HasTextureCoords(0))
                    {
                        vertices.push_back(aiMesh->mTextureCoords[0][i].x);
                        vertices.push_back(aiMesh->mTextureCoords[0][i].y);
                    }
                    else
                    {
                        vertices.push_back(0);
                        vertices.push_back(0);
                    }
                }
                else if (Attributes[j] == VertexAttribute::VA_UV1)
                {
                    if (aiMesh->HasTextureCoords(1))
                    {
                        vertices.push_back(aiMesh->mTextureCoords[1][i].x);
                        vertices.push_back(aiMesh->mTextureCoords[1][i].y);
                    }
                    else
                    {
                        vertices.push_back(0);
                        vertices.push_back(0);
                    }
                }
                else if (Attributes[j] == VertexAttribute::VA_Normal)
                {
                    vertices.push_back(aiMesh->mNormals[i].x);
                    vertices.push_back(aiMesh->mNormals[i].y);
                    vertices.push_back(aiMesh->mNormals[i].z);
                }
                else if (Attributes[j] == VertexAttribute::VA_Tangent)
                {
                    vertices.push_back(aiMesh->mTangents[i].x);
                    vertices.push_back(aiMesh->mTangents[i].y);
                    vertices.push_back(aiMesh->mTangents[i].z);
                    vertices.push_back(1);
                }
                else if (Attributes[j] == VertexAttribute::VA_Color)
                {
                    if (aiMesh->HasVertexColors(i))
                    {
                        vertices.push_back(aiMesh->mColors[0][i].r);
                        vertices.push_back(aiMesh->mColors[0][i].g);
                        vertices.push_back(aiMesh->mColors[0][i].b);
                    }
                    else
                    {
                        vertices.push_back(defaultColor.x);
                        vertices.push_back(defaultColor.y);
                        vertices.push_back(defaultColor.z);
                    }
                }
                else if (Attributes[j] == VertexAttribute::VA_Custom0 ||
                         Attributes[j] == VertexAttribute::VA_Custom1 ||
                         Attributes[j] == VertexAttribute::VA_Custom2 ||
                         Attributes[j] == VertexAttribute::VA_Custom3
                )
                {
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                    vertices.push_back(0.0f);
                }
            }
        }
}

void VulkanModel::LoadIndices(std::vector<uint32>& indices, const aiMesh* aiMesh, const aiScene* aiScene)
{
    for (int32 i = 0; i < (int32)aiMesh->mNumFaces; ++i)
    {
        indices.push_back(aiMesh->mFaces[i].mIndices[0]);
        indices.push_back(aiMesh->mFaces[i].mIndices[1]);
        indices.push_back(aiMesh->mFaces[i].mIndices[2]);
    }
}

void VulkanModel::LoadPrimitives(std::vector<float>& vertices, std::vector<uint32>& indices, Ref<VulkanMesh> mesh,const aiMesh* aiMesh, const aiScene* aiScene)
{
    int32 stride = (int32)vertices.size() / aiMesh->mNumVertices;

    //面数太多了要拆分
    if (indices.size() > 65535)
    {
        std::unordered_map<uint32, uint32> indicesMap;
        Ref<VulkanPrimitive> primitive = nullptr;

        for (int32 i = 0; i < indices.size(); ++i)
        {
            uint32 idx = indices[i];
            if (primitive == nullptr)
            {
                primitive = CreateRef<VulkanPrimitive>();
                indicesMap.clear();
                
                mesh->m_Primitives.push_back(primitive);
            }

            uint32 newIdx = 0;
            auto it = indicesMap.find(idx);
            if (it == indicesMap.end())
            {
                uint32 start = idx * stride;
                newIdx = (uint32)primitive->vertices.size() / stride;
                primitive->vertices.insert(primitive->vertices.end(), vertices.begin() + start, vertices.begin() + start + stride);
                indicesMap.insert(std::make_pair(idx, newIdx));
            }
            else
            {
                newIdx = it->second;
            }

            primitive->indices.push_back(newIdx);

            if (primitive->indices.size() == 65535)
            {
                primitive = nullptr;
            }
        }

        if (CmdBuffer)
        {
            for (int32 i = 0; i < mesh->m_Primitives.size(); ++i)
            {
                primitive = mesh->m_Primitives[i];
                primitive->VertexBuffer = VulkanVertexBuffer::Create(Device, CmdBuffer, primitive->vertices, Attributes);
                primitive->IndexBuffer  = VulkanIndexBuffer::Create(Device, CmdBuffer, primitive->indices);
            }
        }
    }
    else
    {
        Ref<VulkanPrimitive> primitive = CreateRef<VulkanPrimitive>();
        primitive->vertices = vertices;
        for (uint16 i = 0; i < indices.size(); ++i)
        {
            primitive->indices.push_back(indices[i]);
        }
        mesh->m_Primitives.push_back(primitive);

        if (CmdBuffer)
        {
            primitive->VertexBuffer = VulkanVertexBuffer::Create(Device, CmdBuffer, primitive->vertices, Attributes);
            primitive->IndexBuffer  = VulkanIndexBuffer::Create(Device, CmdBuffer, primitive->indices);
        }
    }

    for (int32 i = 0; i < mesh->m_Primitives.size(); ++i)
    {
        Ref<VulkanPrimitive> primitive = mesh->m_Primitives[i];
        primitive->vertexCount  = (int32)primitive->vertices.size() / stride;
        primitive->triangleNum  = (int32)primitive->indices.size() / 3;

        mesh->VertexCount   += primitive->vertexCount;
        mesh->TriangleCount += primitive->triangleNum;
    }
}
