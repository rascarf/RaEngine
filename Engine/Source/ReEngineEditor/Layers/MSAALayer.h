#pragma once
#include "Core/Core.h"
#include "GraphicalLayer.h"
#include "Camera/EditorCamera.h"
#include "Mesh/LineSphere.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/Mesh/VulkanMesh.h"

class MSAABackBuffer : public VulkanBackBuffer
{
public:
    MSAABackBuffer(int32 width, int32 height, const char* title,
                            Ref<VulkanTexture> InMSAAColorRT,
                            Ref<VulkanTexture> InMSAADepthRT,
                            Ref<VulkanTexture> NomalDepthRT,
                            VkSampleCountFlagBits InSamplerCount,
                            bool InbUseMSAA)
    : VulkanBackBuffer(width,height,title),
      MSAAColor(InMSAAColorRT),MSAADepth(InMSAADepthRT),NormalDepth(NomalDepthRT),bUseMSAA(InbUseMSAA)
    {
        m_SampleCount = InSamplerCount;
    }

public:

    Ref<VulkanTexture> MSAADepth;
    Ref<VulkanTexture> MSAAColor;
    Ref<VulkanTexture> NormalDepth;
    bool bUseMSAA = false;
    
    virtual void CreateFrameBuffers() override;
    virtual void CreateRenderPass() override;

    virtual void DestroyFrameBuffers() override;
};

class MSAALayer : public GraphicalLayer
{
public:
    virtual void OnEvent(std::shared_ptr<ReEngine::Event> e) override;
    virtual void OnUpdate(ReEngine::Timestep ts) override;

    virtual void OnRender() override;
    virtual void OnUIRender(ReEngine::Timestep ts) override;

    virtual void OnCreateBackBuffer() override;
    virtual void OnInit() override;
    virtual void OnDeInit() override;
    virtual void OnChangeWindowSize(std::shared_ptr<ReEngine::Event> e);

    void CreateBuffer();

public:

    VkSampleCountFlagBits  MSAACount = VK_SAMPLE_COUNT_8_BIT;
    Ref<VulkanTexture> MSAAColorTexture = nullptr;
    Ref<VulkanTexture> MSAADepthTexture = nullptr;
    Ref<VulkanTexture> NormalDepth = nullptr;
    bool MSAAEnable = false;

    Ref<VulkanModel> LineModel = nullptr;
    Ref<VulkanShader> LineShader = nullptr;
    Ref<VulkanMaterial> MSAAMaterial = nullptr;

    Ref<EditorCamera>                               m_Camera = nullptr;        
    Ref<VulkanDynamicBufferRing>                    m_RingBuffer = nullptr;

    MVPBlock MVPData;
};
