#pragma once
#include "GraphicalLayer.h"
#include "Camera/EditorCamera.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/Mesh/VulkanMesh.h"


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

    ViewProjectionBlock    m_VPData;
    ModelBlock             m_ModelData;

    AttachmentParamBlock DebugParam;

    Ref<EditorCamera>                               m_Camera = nullptr;        
    Ref<VulkanDynamicBufferRing>                    m_RingBuffer = nullptr;
};
