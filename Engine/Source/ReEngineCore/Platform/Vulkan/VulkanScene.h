#pragma once

#include "VulkanContext.h"
#include "core/Application.h"
#include "Core/Core.h"
#include "Core/SIngletonTemplate.h"
#include "entt/include/entt.hpp"
#include "FrameWork/Scene.h"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "Mesh/VulkanMesh.h"
#include "Mesh/AssetLoader.h"
#include "Mesh/AsyncLoader.h"
#include "RenderGraph/RenderGraph.h"

namespace ReEngine
{
    
    class VulkanScene : public SingletonTemplate<VulkanScene>
    {
    public:
        VulkanScene(VulkanContext* Context)
        {
            Context->Instance->GetDevice()->mContext = Context;
            mDevice = Context->GetVulkanInstance()->GetDevice();
            
            m_ASyncLoader = CreateRef<AsyncLoader>();
            m_ASyncLoader->Init(mDevice);

            m_FrameGraphBuilder = CreateRef<FrameGraphBuilder>();
            m_FrameGraphBuilder->Init(Context);

            m_FrameGraph = CreateRef<FrameGraph>();
            m_FrameGraph->Init(m_FrameGraphBuilder.get());

            mDevice.reset();
        };
        
        ~VulkanScene()
        {
            m_ASyncLoader->ShutDown();
            m_FrameGraph->ShutDown();
            m_FrameGraphBuilder->ShutDown();
            
            m_FrameGraph.reset();
            m_FrameGraphBuilder.reset();
            m_ASyncLoader.reset();
            mDevice.reset();
        };
        
        void Update(Timestep ts)
        {
            
            m_ASyncLoader->Update_MainThread();
        }

    public: // ------------------ Entity -----------------
        Ref<Entity> CreateEntity(const std::string& name,Ref<Entity> Parent = nullptr);
        Ref<Entity> FindSameNameEntity(const std::string);
        
    public: // ------------------ Asset ----------------
        void ReqeustLoadGLTF();
        void ReqeustLoadModel(const ModelLoadRequest& ModelRequest);
        
        void HandleLoadList(Ref<VulkanDevice> vulkanDevice, Ref<VulkanCommandBuffer> cmdBuffer);
        void ImportModel(Ref<VulkanModel> Model);
        void ImportTexture(int32 MaterialIndex);

    public:// ------------------ RenderScene ----------------
        void ReqeustParseFrameGraph(std::string FileName);
        void RegisterPass(std::string PassName,Ref<FrameGraphRenderPass> RenderPass);
    public:
        std::vector<ModelLoadRequest> ModelToLoad;
        
        std::vector<Ref<VulkanTexture>> Textures;
        std::vector<VulkanMaterialInfo> PBRTextureParam;

    public:
        std::vector<Ref<Entity>> EntityNodes;
        Scene  mScene;
        
        std::vector<Ref<VulkanMesh>> VulkanMeshs;
        std::vector<Ref<VulkanModel>> VulkanModels;

        Ref<AsyncLoader>              m_ASyncLoader = nullptr;
        Ref<FrameGraph>               m_FrameGraph = nullptr;
        Ref<FrameGraphBuilder>        m_FrameGraphBuilder = nullptr;
        Ref<VulkanDevice>             mDevice = nullptr;

        Ref<EditorCamera>             SceneCamera = nullptr;
    };    
}

