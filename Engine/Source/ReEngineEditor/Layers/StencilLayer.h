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
    void GenerateDiffuseAndSpecular();
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

    Ref<VulkanTexture>      PreFilterCubeMap;
    Ref<VulkanShader>       SpcularFilterShader;       
    Ref<VulkanComputePipeline> SpcularFilterPipe;
    Ref<VulkanTexture>      EnvCubeMap;

    Ref<VulkanTexture>      IrraMap;
    Ref<VulkanShader>       IrraShader;       
    Ref<VulkanComputePipeline> IrraPipe;

    Ref<VulkanTexture>      Lut;
    Ref<VulkanShader>       LutShader;       
    Ref<VulkanComputePipeline> LutPipe;

    Ref<VulkanModel>        PBRModel;
    std::vector<Ref<VulkanTexture>> PBRTextureArray;
    Ref<VulkanShader>       PBRShader;
    Ref<VulkanMaterial>     PBRPipeline;

    Ref<VulkanShader>           SHFilterShader;
    Ref<VulkanComputePipeline> SHFilterPipe;
    Ref<VulkanBuffer>       SHReadBackBuffer;

    struct SHFilterParam
    {
        int32                   ci = 0;
        int32                   BatchingIndex = 0;
        int32                   TotalSampleCount = 1024 * 1024;
    }SHParam;
    
    struct ModelViewProjectionBlock
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    }m_MVPData;

    struct SpecularFilterPushConstants
    {
        uint32_t level;
        float roughness;
    };

    struct AnalyticalLight {
        glm::vec4 direction;
        glm::vec4 radiance;
    };

    // 这个应该以SpecializationInfo的方式传入Shader
    static const int NumLights = 3;
    struct ShadingUniforms
    {
        AnalyticalLight lights[NumLights];
        glm::vec4 eyePosition;
    }LightParams;
    
    AttachmentParamBlock DebugParam;

    Ref<EditorCamera>                               m_Camera = nullptr;        
    Ref<VulkanDynamicBufferRing>                    m_RingBuffer = nullptr;
    Ref<AsyncLoader>                                m_ASyncLoader = nullptr;
    VulkanSpecialInfo                               vkInfo;
};
