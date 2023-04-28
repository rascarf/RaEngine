#pragma once
#include "GraphicalLayer.h"
#include "Camera/EditorCamera.h"
#include "Platform/Vulkan/Mesh/VulkanMesh.h"

class InputAttachmentBackBuffer : public VulkanBackBuffer
{
public:
    InputAttachmentBackBuffer(int32 width, int32 height, const char* title,
                            std::vector<Ref<VulkanTexture>> AttachmentDepth,
                            std::vector<Ref<VulkanTexture>> AttachmentNormals,
                            std::vector<Ref<VulkanTexture>> AttachmentColors)
    : VulkanBackBuffer(width,height,title),
    m_AttachmentColors(AttachmentColors),
    m_AttachmentDepth(AttachmentDepth),
    m_AttachmentNormals(AttachmentNormals)
    {
    }
public:
    virtual void CreateFrameBuffers() override;
    virtual void CreateRenderPass() override;

public:
    std::vector<Ref<VulkanTexture>>             m_AttachmentDepth;
    std::vector<Ref<VulkanTexture>>             m_AttachmentNormals;
    std::vector<Ref<VulkanTexture>>             m_AttachmentColors;
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
    void CreateDescriptor();
    void CreateAttachments();
    void CreatePipeline();
    
protected:

    Ref<EditorCamera>                               m_Camera = nullptr;        
    
    Ref<VulkanDynamicBufferRing>                    m_RingBuffer = nullptr;
    
    Ref<VulkanModel>                                m_Model = nullptr;
    Ref<VulkanModel>                                m_Quad = nullptr;

    Ref<VulkanPipeline>                             m_Pipeline0 = nullptr;
    Ref<VulkanShader>                               m_Shader0 = nullptr;
    Ref<VulkanDescriptorSet>                        m_DescriptorSet0 = nullptr;

    Ref<VulkanPipeline>                             m_Pipeline1 = nullptr;
    Ref<VulkanShader>                               m_Shader1 = nullptr;
    std::vector<Ref<VulkanDescriptorSet>>           m_DescriptorSets;
    
    std::vector<Ref<VulkanTexture>>                 m_AttachmentDepth;
    std::vector<Ref<VulkanTexture>>                 m_AttachmentNormals;
    std::vector<Ref<VulkanTexture>>                 m_AttachmentColors;

    ModelBlock ModelMatrix;
    AttachmentParamBlock m_DebugParam;
    std::vector<const char*> m_DebugNames;
    ViewProjectionBlock ViewParam;
};


