#pragma once
#include "Core/Core.h"
#include "ReEngineEditor/Layers/GraphicalLayer.h"
#include "Camera/EditorCamera.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/Mesh/VulkanMesh.h"

class AnimationLayer : public GraphicalLayer
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

    float                       m_AnimDuration = 0.0f;
    float                       m_AnimTime = 0.0f;
    
    struct ModelViewProjectionBlock
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    }m_MVPData;

#define MAX_BONES 64
    struct BonesTransformBlock
    {
        glm::mat4 bones[MAX_BONES];
    }m_BonesData;

    AttachmentParamBlock DebugParam;

    Ref<EditorCamera>                               m_Camera = nullptr;        
    Ref<VulkanDynamicBufferRing>                    m_RingBuffer = nullptr;
};
