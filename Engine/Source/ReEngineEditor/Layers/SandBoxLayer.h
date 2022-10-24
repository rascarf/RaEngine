#pragma once

#include "ReEngineEditor/Editor/Panels/SceneHierarchyPanel.h"
#include "Camera/OrthogaraphicCameraController.h"
#include "ECSFrameWork/Entity/Entity.h"
#include "Library/ShaderLibrary.h"
#include "ReEngineCore/Layer/Layer.h"
#include "Renderer/RHI/Shader.h"
#include "Renderer/RHI/Texture.h"
#include "Renderer/RHI/VertexArray.h"
#include "Renderer/RHI/FrameBuffer.h"
struct FramebufferSpecification
{
    uint32_t Width, Height;
    // FramebufferFormat Format = 
    uint32_t Samples = 1;

    bool SwapChainTarget = false;
};

class SandBoxLayer : public ReEngine::Layer
{
public:
    SandBoxLayer();
    virtual ~SandBoxLayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual void OnEvent(std::shared_ptr<ReEngine::Event> e) override;
    virtual void OnUpdate(ReEngine::Timestep ts) override;

    virtual void OnUIRender(ReEngine::Timestep ts) override;

private:
    ReEngine::Ref<ReEngine::FrameBuffer> mFrameBuffer;

    ReEngine::Entity mCameraEntity;
    ReEngine::Ref<ReEngine::Scene> mActiveScene;
    ReEngine::Entity mRenderEntity;

    //Panels
    ReEngine::SceneHierarchyPanel SceneHierarchyPanel;

    glm::vec2 mViewPortSize{0.0,0.0};
};


