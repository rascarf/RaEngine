#pragma once
#include "Core/Core.h"
#include "ReEngineEditor/Layers/GraphicalLayer.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/Mesh/VulkanMesh.h"
#include "Camera/EditorCamera.h"

class ComputeLayer : public GraphicalLayer
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
    void ProcessImage();
    
private:
    bool mReady = false;

    Ref<VulkanModel> PlaneModel = nullptr;
    Ref<VulkanMaterial> mMaterial = nullptr;
    Ref<VulkanShader> mShader = nullptr;
    Ref<VulkanTexture> mTexture = nullptr;
    
    std::vector<Ref<VulkanTexture>> ComputeTargets;
    std::vector<Ref<VulkanShader>> ComputeShaders;
    std::vector<Ref<VulkanComputeMaterial>> ComputeMaterial;

    std::vector<const char*> m_FilterNames;
    int32 m_FilterIndex;

private:
    Ref<VulkanTexture>                              ColorRT;
    Ref<VulkanTexture>                              DepthRT;
    Ref<VulkanRenderTarget>                         RenderTarget;

    Ref<VulkanModel>                                mQuad = nullptr;
    Ref<VulkanMaterial>                             mFilterMaterial;
    Ref<VulkanShader>                               mFilterShader;

    MVPBlock                                        m_MVPData;
    Ref<EditorCamera>                               m_Camera = nullptr;        
    Ref<VulkanDynamicBufferRing>                    m_RingBuffer = nullptr;
};
