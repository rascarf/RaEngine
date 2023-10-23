#pragma once
#include "Core/Core.h"
#include "ReEngineEditor/Layers/GraphicalLayer.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/Mesh/VulkanMesh.h"
#include "Camera/EditorCamera.h"
#include "Platform/Vulkan/RayTracing/RTPipelineInfo.h"
#include "Platform/Vulkan/RayTracing/Scene/SimpleglTFScene.h"

struct GlobalParamBlock
{
    glm::vec4 Pos;
    glm::mat4 InvProj;
    glm::mat4 InvView;

    glm::vec4 SamplingData;
    glm::vec4 ViewSize;

    glm::vec4 LightInfo;
    glm::vec4 moving;
};


class SimplePathTracing : public GraphicalLayer
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
    void PrepreUniformBuffers();
    void CreateMaterials();
    
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

    RTPipelineInfo                                  RaytracingPipeline;

    Ref<VulkanShader>                               m_CloseHitShader = nullptr;
    Ref<VulkanBuffer>                               mLightBuffer = nullptr;
    Ref<VulkanBuffer>                               mMaterialBuffer = nullptr;
    Ref<VulkanBuffer>                               mObjectBuffer = nullptr;
    Ref<VulkanTexture>                              mStorageImage = nullptr;
    GlobalParamBlock                                m_GlobalParam;

public:

    glTFScene                                       Scene;
};
