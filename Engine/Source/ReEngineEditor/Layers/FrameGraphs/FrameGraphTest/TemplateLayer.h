#pragma once
#include "Core/Core.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/Mesh/VulkanMesh.h"
#include "Camera/EditorCamera.h"
#include "Platform/Vulkan/RenderGraph/RenderGraph.h"
#include "ReEngineEditor/Layers/GraphicalLayer.h"

class FrameGraphTemplateLayer : public GraphicalLayer
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
    void LoadAsset();
    
private:

    Ref<VulkanModel>                                mQuad = nullptr;
    Ref<VulkanMaterial>                             mFilterMaterial;
    Ref<VulkanShader>                               mFilterShader;

    MVPBlock                                        m_MVPData;
    Ref<EditorCamera>                               m_Camera = nullptr;        
    Ref<VulkanDynamicBufferRing>                    m_RingBuffer = nullptr;

    FrameGraphBuilder                               mFrameGraphBuilder;
    FrameGraph                                      mFrameGraph;
    
};
