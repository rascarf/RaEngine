#pragma once

#include "ReEngineEditor/Editor/Panels/SceneHierarchyPanel.h"
#include "Camera/OrthogaraphicCameraController.h"
#include "FrameWork/Entity/Entity.h"
#include "Library/ShaderLibrary.h"
#include "Platform/Vulkan/VulkanPipelineInfo.h"
#include "Platform/Vulkan/Mesh/VulkanMesh.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanDynamicBufferRing.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanFrameBuffer.h"
#include "ReEngineCore/Layer/Layer.h"
#include "Renderer/RHI/Shader.h"
#include "Renderer/RHI/Texture.h"
#include "Renderer/RHI/VertexArray.h"
#include "Renderer/RHI/FrameBuffer.h"

class SandBoxLayer : public ReEngine::Layer
{
public:

    struct UniformBufferObject 
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    struct ParamBlock
    {
        glm::vec3 lightDir;
        float curvature;

        glm::vec3 lightColor;
        float exposure;

        glm::vec2 curvatureScaleBias;
        float blurredLevel;
        float padding;
    };
    
    SandBoxLayer();
    virtual ~SandBoxLayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual void OnEvent(std::shared_ptr<ReEngine::Event> e) override;
    virtual void OnUpdate(ReEngine::Timestep ts) override;

    virtual void OnRender() override;
    virtual void OnUIRender(ReEngine::Timestep ts) override;

    
    Ref<VulkanBackBuffer> FrameBuffer;
    Ref<VulkanPipeline> GraphicsPipeline;
    Ref<VulkanDynamicBufferRing> RingBuffer;
    Ref<VulkanShader> PipeShader;
    Ref<VulkanDescriptorSet> PipeSet;
        
    Ref<VulkanModel> Model;
    Ref<VulkanTexture> TexDiffuse;
    Ref<VulkanTexture> TexNomal;
    Ref<VulkanTexture> TexPreIntegareted;
    Ref<VulkanTexture> TexCurve;
    Ref<EditorCamera> Camera;

    ReEngine::VulkanContext* VkContext;

    void CreateGraphicsPipeline();
    void CreateMeshBuffer();
    void UpdateUniformBuffer(Timestep ts);
private:
    UniformBufferObject ubo;
    ParamBlock Param;
};


