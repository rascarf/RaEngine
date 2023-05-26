#pragma once
#include "GraphicalLayer.h"
#include "Camera/EditorCamera.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/Mesh/VulkanMesh.h"

#define NUM_LIGHTS 64

struct PointLight
{
    glm::vec4 Position;
    glm::vec3 Color;
    float Radius;
};

struct LightSpawnBlock
{
    glm::vec3 Position[NUM_LIGHTS];
    glm::vec3 Direction[NUM_LIGHTS];
    float Speed[NUM_LIGHTS];
};

struct LightDataBlock
{
    PointLight Lights[NUM_LIGHTS];
};

class InputAttachmentBackBuffer : public VulkanBackBuffer
{
public:
    InputAttachmentBackBuffer(int32 width, int32 height, const char* title,
                            std::vector<Ref<VulkanTexture>>* AttachmentDepth,
                            std::vector<Ref<VulkanTexture>>* AttachmentNormals,
                            std::vector<Ref<VulkanTexture>>* AttachmentColors)
    : VulkanBackBuffer(width,height,title),
    m_AttachmentColors(AttachmentColors),
    m_AttachmentDepth(AttachmentDepth),
    m_AttachmentNormals(AttachmentNormals)
    {
    }
public:
    virtual void CreateFrameBuffers() override;
    virtual void CreateRenderPass() override;
    virtual void CreateDepthStencil() override{};
public:
    std::vector<Ref<VulkanTexture>>*             m_AttachmentDepth;
    std::vector<Ref<VulkanTexture>>*             m_AttachmentNormals;
    std::vector<Ref<VulkanTexture>>*             m_AttachmentColors;
};

struct ModelBlock
{
    glm::mat4 model;
};

struct ViewProjectionBlock
{
    glm::mat4 view;
    glm::mat4 projection;
};

struct AttachmentParamBlock
{
    int attachmentIndex;
    float zNear;
    float zFar;
    float padding;
};

struct RayParamBlock
{
    glm::vec3    color;
    float        power;
    glm::vec3    viewDir;
    float        padding;
};

class InputAttachment : public GraphicalLayer
{
public:
    virtual void OnCreateBackBuffer() override;
    virtual void OnInit() override;
    virtual void OnDeInit() override;

    virtual void OnUpdate(Timestep ts) override;
    virtual void OnRender() override;
    virtual void OnUIRender(Timestep ts) override;
    virtual void OnChangeWindowSize(std::shared_ptr<ReEngine::Event> e) override;

public:
    void CreateBuffers();
    void CreateMaterial();
    void CreateAttachments();


private:
    void UpdateLight(Timestep ts);
    
protected:

    Ref<EditorCamera>                               m_Camera = nullptr;        
    
    Ref<VulkanDynamicBufferRing>                    m_RingBuffer = nullptr;
    
    Ref<VulkanModel>                                m_Model = nullptr;
    std::vector<Ref<VulkanTexture>>                 m_ModelDiffuses;
    std::vector<std::vector<Ref<VulkanMesh>>>       m_SceneMeshes;
    
    
    Ref<VulkanMaterial>                             m_Material0 = nullptr;
    Ref<VulkanShader>                               m_Shader0 = nullptr;
    
    Ref<VulkanModel>                                m_Quad = nullptr;
    Ref<VulkanMaterial>                             m_Material1 = nullptr;
    Ref<VulkanShader>                               m_Shader1 = nullptr;

    Ref<VulkanModel>                                m_Role = nullptr;
    Ref<VulkanTexture>                              m_RoleDiffuse = nullptr;
    Ref<VulkanMaterial>                             m_RoleMaterial = nullptr;
    
    glm::vec3                                       m_RolePosition;
    glm::vec3                                       m_RoleScale;
    
    Ref<VulkanMaterial>                             m_EffectMaterial1 = nullptr;
    Ref<VulkanShader>                               m_EffectShader = nullptr;
    
    std::vector<Ref<VulkanTexture>>                 m_AttachmentDepth;
    std::vector<Ref<VulkanTexture>>                 m_AttachmentNormals;
    std::vector<Ref<VulkanTexture>>                 m_AttachmentColors;

    ModelBlock ModelMatrix;
    ViewProjectionBlock ViewParam;
    
    LightDataBlock LightDatas;
    LightSpawnBlock LightInfos;
    
    AttachmentParamBlock m_DebugParam;
    RayParamBlock        m_RayData;
    std::vector<const char*> m_DebugNames;

    float CurrentTime;
};


