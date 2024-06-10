#include "VulkanScene.h"
#include "Component/StaticMeshComponent.h"
#include "FrameWork/Component/TransformComponent.h"
#include "yaml-cpp/src/exp.h"

namespace ReEngine
{
    Ref<Entity> VulkanScene::CreateEntity(const std::string& name,Ref<Entity> Parent)
    {
        Ref<Entity> CreateEntity =  mScene.CreateEntity(name);
        EntityNodes.emplace_back(CreateEntity);

        if(Parent)
        {
            CreateEntity->mParent = Parent;
            
            Parent->mChildren.emplace_back(CreateEntity);
        }

        return CreateEntity;
    }

    void VulkanScene::ReqeustLoadGLTF()
    {
        
    }

    void VulkanScene::ReqeustLoadModel(const ModelLoadRequest& ModelRequest)
    {
        ModelToLoad.emplace_back(ModelRequest);
    }

    void VulkanScene::ReqeustParseFrameGraph(std::string FileName)
    {
        m_FrameGraph->Parse(FileName.c_str());
    }

    void VulkanScene::RegisterPass(std::string PassName, Ref<FrameGraphRenderPass> RenderPass)
    {
        m_FrameGraphBuilder->RegisterRenderPass(PassName,RenderPass);
    }

    void VulkanScene::HandleLoadList(Ref<VulkanDevice> vulkanDevice, Ref<VulkanCommandBuffer> cmdBuffer)
    {
        std::map<std::string,ModelLoadRequest> ToLoadMap;

        //遍历所有 去重
        for (int32 i = 0 ; i < ModelToLoad.size() ; i++)
        {
            if(ToLoadMap.contains(ModelToLoad[i].filename))
            {
                for(auto& Attribute: ModelToLoad[i].VertexAttributes)
                {
                    ToLoadMap[ModelToLoad[i].filename].VertexAttributes.emplace(Attribute);
                }
                
                continue;
            }

            ToLoadMap.try_emplace(ModelToLoad[i].filename,ModelToLoad[i]);
        }

        for(auto& Request : ToLoadMap)
        {
            std::vector<VertexAttribute> Attributes {};
            for(auto& Attribute : Request.second.VertexAttributes)
            {
                Attributes.emplace_back(Attribute);
            }

            auto VulkanModel = VulkanModel::LoadFromFile(Request.second.filename,vulkanDevice,cmdBuffer,Attributes,true);
            VulkanModel->Name = Request.first;
            ImportModel(VulkanModel);
            
            VulkanModels.emplace_back(VulkanModel);
        }
    }

    void VulkanScene::ImportModel(Ref<VulkanModel> Model)
    {
        if(Model)
        {
            Ref<Entity> ModelEntity = CreateEntity(Model->Name,nullptr);
            
            std::vector<Ref<VulkanMeshNode>> Nodes;

            // LoadMesh
            for(auto& Mesh: Model->Meshes)
            {
                if(!Mesh->LinkNode.expired())
                {
                    Nodes.emplace_back(Mesh->LinkNode);    
                }
            }
            
            while(!Nodes.empty())
            {
                auto TopNode = Nodes.back();
                Nodes.pop_back();

                for(auto& Mesh : TopNode->Meshes)
                {
                    Ref<Entity> Entity =  CreateEntity(TopNode->name,ModelEntity);
                    Entity->AddComponent<StaticMeshComponent>(Mesh);
                    Entity->GetComponent<TransformComponent>().LocalMatrix = TopNode->GetGlobalMatrix();
                }
            }

            // 处理Material
            for(auto& Mesh : Model->Meshes)
            {
                bool FindSame = false;
                for(int i = 0 ; i < PBRTextureParam.size() ; ++i)
                {
                    const auto& Material = PBRTextureParam[i];
                    if(Material.Diffuse == Mesh->Material.Diffuse
                    && Material.Metalic == Mesh->Material.Metalic
                    && Material.Normal == Mesh->Material.Normal
                    && Material.Specular == Mesh->Material.Specular)
                    {
                        FindSameNameEntity(Mesh->LinkNode.lock()->name)->GetComponent<StaticMeshComponent>().PbrMaterialParamIndex = i;
                        FindSame = true;
                        break;
                    }

                    if(FindSame)
                    {
                        break;
                    }
                }

                if(!FindSame)
                {
                    const auto BackIndex = PBRTextureParam.size();
                    PBRTextureParam.emplace_back(Mesh->Material);
                    
                    ImportTexture(BackIndex);
                    FindSameNameEntity(Mesh->LinkNode.lock()->name)->GetComponent<StaticMeshComponent>().PbrMaterialParamIndex = BackIndex;
                }
            }
        }
    }

    void VulkanScene::ImportTexture(int32 MaterialIndex)
    {
        auto Info = &PBRTextureParam[MaterialIndex];
        
        if(!Info->Diffuse.empty())
        m_ASyncLoader->RequestTexture(Info->Diffuse + ".jpg",[MaterialIndex,this](FileLoadCallBackParamStruct& param)
        {
            if(param.TextureHanle)
            {
                auto Info = &PBRTextureParam[MaterialIndex];
                
                Info->DiffuseTexture = param.TextureHanle;
                Info->DiffuseTexture.lock()->Name = param.FilePath;
                Info->DiffuseTextureIndex = Textures.size();
                
                Textures.emplace_back(param.TextureHanle);
            }
        });

        if(!Info->Specular.empty())
        m_ASyncLoader->RequestTexture(Info->Specular + ".jpg",[&Info,this](FileLoadCallBackParamStruct& param)
        {
            if(param.TextureHanle)
            {
                Info->SpecularTexture = param.TextureHanle;
                Info->SpecularTextureIndex = Textures.size();
                Textures.emplace_back(param.TextureHanle);
                param.TextureHanle->Name = param.FilePath;
            }
        });

        if(!Info->Normal.empty())
        m_ASyncLoader->RequestTexture(Info->Normal + ".jpg",[&Info,this](FileLoadCallBackParamStruct& param)
        {
            if(param.TextureHanle)
            {
                Info->NormalTextureIndex = Textures.size();
                Info->NormalTexture = param.TextureHanle;
                
                Textures.emplace_back(param.TextureHanle);
                param.TextureHanle->Name = param.FilePath;
            }
        });

        if(!Info->Metalic.empty())
        m_ASyncLoader->RequestTexture(Info->Metalic + ".jpg",[&Info,this](FileLoadCallBackParamStruct& param)
        {
            if(param.TextureHanle)
            {
                Info->MetalicTexture = param.TextureHanle;
                Info->MetalicTextureIndex = Textures.size();
                
                Textures.emplace_back(param.TextureHanle);
                param.TextureHanle->Name = param.FilePath;
            }
        });
    }

    Ref<Entity> VulkanScene::FindSameNameEntity(const std::string EntityName)
    {
        for(auto& Entity : EntityNodes)
        {
            if(Entity->GetComponent<TagComponent>().Tag == EntityName)
            {
                return Entity;
            }
        }

        return nullptr;
    }
}


