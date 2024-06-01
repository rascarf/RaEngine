#pragma once
#include "Core/Core.h"
#include "GraphicalLayer.h"
#include "Platform/Vulkan/VulkanGraphicPipeline.h"
#include "Platform/Vulkan/Mesh/VulkanMesh.h"
#include "Camera/EditorCamera.h"
#include "Platform/Vulkan/VulkanScene.h"

class TemplateLayer : public GraphicalLayer
{
public:
    virtual void OnCreateBackBuffer() override;
    virtual void OnInit() override;
    virtual void OnDeInit() override;

    virtual void OnUpdate(Timestep ts) override;
    virtual void OnRender() override;
    virtual void OnUIRender(Timestep ts) override;
    virtual void OnChangeWindowSize(std::shared_ptr<ReEngine::Event> e) override;

    virtual void OnNativeUpdate(Timestep ts){ /*子类里面写自己的代码*/ }
    virtual void OnNativeLoadAsset(){ /*子类里面写自己的代码*/ }
    virtual void OnNativeDeInit(){/*子类里面写自己的代码*/ }
private:
    void CreateRenderTarget();
    void LoadAsset();
    
private:
    Ref<VulkanTexture>                              ColorRT;
    Ref<VulkanTexture>                              DepthRT;
    Ref<VulkanRenderTarget>                         RenderTarget;

    Ref<VulkanModel>                                mQuad = nullptr;
    Ref<VulkanGraphicPipeline>                             mFilterMaterial;
    Ref<VulkanShader>                               mFilterShader;

    MVPBlock                                        m_MVPData;
    Ref<EditorCamera>                               m_Camera = nullptr;        
    Ref<VulkanDynamicBufferRing>                    m_RingBuffer = nullptr;

public:
    Ref<VulkanScene>                                m_Scene = nullptr;
};
