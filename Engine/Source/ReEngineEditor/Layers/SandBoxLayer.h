#pragma once
#include "GraphicalLayer.h"
#include "Camera/EditorCamera.h"
#include "Platform/Vulkan/VulkanPipelineInfo.h"
#include "Platform/Vulkan/Mesh/VulkanMesh.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanDynamicBufferRing.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanFrameBuffer.h"

class SandBoxLayer : public GraphicalLayer
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
    
    virtual void OnUpdate(ReEngine::Timestep ts) override;
    virtual void OnRender() override;
    virtual void OnUIRender(ReEngine::Timestep ts) override;
    virtual void OnChangeWindowSize(std::shared_ptr<ReEngine::Event> e) override;

    virtual void OnInit() override;
    virtual void OnDeInit() override;
    
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
    
    void CreateGraphicsPipeline();
    void CreateMeshBuffer();
    void UpdateUniformBuffer(Timestep ts);
private:
    UniformBufferObject ubo;
    ParamBlock Param;
};


