#pragma once
#include "Camera/EditorCamera.h"
#include "Core/Core.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/VulkanRenderTarget.h"
#include "Platform/Vulkan/Mesh/VulkanMesh.h"
#include "ReEngineEditor/Layers/GraphicalLayer.h"

#define LIGHT_SIZE 512
#define TILE_SIZE 16
#define LIGHT_SIZE_PER_TILE 63

struct LightsParamBlock
{
    glm::vec4 Count = glm::vec4(0.0,0.0,0.0,0.0);
    GraphicalPointLight Lights[LIGHT_SIZE];
};

struct LightsInfo
{
    glm::vec3 Position[LIGHT_SIZE];
    glm::vec3 Direction[LIGHT_SIZE];
};

struct LightVisiblity
{
    uint32 Count;
    uint32 LightIndices[LIGHT_SIZE_PER_TILE];
};

struct CullingParamBlock
{
    glm::mat4x4 invViewProj;
    glm::vec4 FrameSize;
    glm::vec4 TileNum;
    glm::vec4 Pos;
};

class TileBasedForwardLayer : public GraphicalLayer
{
public:
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

    void InitLightParams();
    void UpdateLights(Timestep ts);
    
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

private:
    bool mReady = false;
    Ref<VulkanModel>             Model;
    Ref<VulkanShader>            ModelShader;
    Ref<VulkanMaterial>          ModelMaterial;

    // PreDepth
    Ref<VulkanShader>            PreDepthShader;
    Ref<VulkanMaterial>          PreDepthMaterial;
    Ref<VulkanTexture>           PreDepthTexture;
    Ref<VulkanRenderTarget>      PreDepthRenderTarget;

    // Compute Pass
    Ref<VulkanShader>            ComputeShader;
    Ref<VulkanComputeMaterial>   ComputeProcessor;
    Ref<VulkanBuffer>            LightCullingBuffer;

    glm::vec4                    Debug;

    LightsInfo                   mLightInfo;
    LightsParamBlock             LightParam;
    CullingParamBlock            CullingParam;
    ModelBlock                   mMVPParam;

    float                        ElapsedTimer = 0;

    int32                        TileCountPerRow = 0;
    int32                        TileCountPerColumn = 0;
};
