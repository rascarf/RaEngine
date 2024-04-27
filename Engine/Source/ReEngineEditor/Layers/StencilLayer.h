#pragma once
#include "GraphicalLayer.h"
#include "Camera/EditorCamera.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/Mesh/AsyncLoader.h"
#include "Platform/Vulkan/Mesh/VulkanMesh.h"
#include "Platform/Vulkan/Mesh/AsyncLoader.h"


class RTLayer : public GraphicalLayer
{
public:
    virtual void OnCreateBackBuffer() override;
    virtual void OnInit() override;
    virtual void OnDeInit() override;

    virtual void OnUpdate(Timestep ts) override;
    virtual void OnRender() override;
    virtual void OnUIRender(Timestep ts) override;
    virtual void OnChangeWindowSize(std::shared_ptr<ReEngine::Event> e) override;

private:

    void CreateRenderTarget();
    void LoadAsset();
private:
    
    Ref<VulkanModel> mQuad = nullptr;
    Ref<VulkanMaterial> mFilterMaterial;
    Ref<VulkanShader> mFilterShader;

    Ref<VulkanRenderTarget> RenderTarget;
    Ref<VulkanTexture> ColorRT;
    Ref<VulkanTexture> DepthRT;

    Ref<VulkanModel> SceneModel;
    Ref<VulkanShader> SceneShader;
    Ref<VulkanMaterial> SceneMaterial;
    
    std::vector<Ref<VulkanTexture>> TextureArray;

    std::vector<std::vector<Ref<VulkanMesh>>> RenderObject;

    Ref<VulkanShader>       SkyBoxShader;
    Ref<VulkanMaterial>     SkyBoxMaterial;
    Ref<VulkanModel>        SkyBox;
    Ref<VulkanTexture>      CubeMap; 
    Ref<VulkanRenderTarget> SkyBoxRenderTarget;

    Ref<VulkanTexture>      EnvTextureEquirect;
    Ref<VulkanShader>       EnvEquirect2CubeShader;       
    Ref<VulkanComputePipeline> EnvEquirect2CubePipe;
    Ref<VulkanTexture>      EnvCubeMap;

    struct ModelViewProjectionBlock
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    }m_MVPData;
    
    AttachmentParamBlock DebugParam;

    Ref<EditorCamera>                               m_Camera = nullptr;        
    Ref<VulkanDynamicBufferRing>                    m_RingBuffer = nullptr;
    Ref<AsyncLoader>                                m_ASyncLoader = nullptr;
};
