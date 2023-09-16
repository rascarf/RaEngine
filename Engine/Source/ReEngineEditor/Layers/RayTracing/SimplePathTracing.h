#pragma once
#include "Core/Core.h"
#include "ReEngineEditor/Layers/GraphicalLayer.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/Mesh/VulkanMesh.h"
#include "Camera/EditorCamera.h"
#include "Platform/Vulkan/RayTracing/Scene/SimpleglTFScene.h"

struct VkGeometryInstance
{
    float Transform[12];

    uint32_t IntanceID : 24;
    uint32_t mask :8;

    uint32_t InstanceOffset : 24; // HitGroupIndex R_I

    uint32_t Flags : 8;
    uint64_t AccelerationStructreHandle;
};

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

struct AccelerationStructureInstance
{
    VkDeviceMemory memory = VK_NULL_HANDLE;

    VkAccelerationStructureKHR AccelerationStruct = VK_NULL_HANDLE;
    uint64 Handle = 0;
};

struct ObjectInstance
{
    glm::ivec4 Params = glm::ivec4(-1,-1,-1,-1);
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


public:

    glTFScene                                       Scene;
};
