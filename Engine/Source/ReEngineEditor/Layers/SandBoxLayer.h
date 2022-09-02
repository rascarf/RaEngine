#pragma once

#include "Camera/OrthogaraphicCameraController.h"
#include "Library/ShaderLibrary.h"
#include "ReEngineCore/Layer/Layer.h"
#include "Renderer/RHI/Shader.h"
#include "Renderer/RHI/Texture.h"
#include "Renderer/RHI/VertexArray.h"


class SandBoxLayer : public ReEngine::Layer
{
public:
    SandBoxLayer();
    virtual ~SandBoxLayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual void OnEvent(std::shared_ptr<ReEngine::Event> e) override;
    virtual void OnUpdate(ReEngine::Timestep ts) override;

private:
    ReEngine::Ref<ReEngine::Library<ReEngine::Shader>> m_ShaderLibrary;
    
    ReEngine::Ref<ReEngine::Shader> mShader;
    ReEngine::Ref<ReEngine::Shader> mBlueShader;
    ReEngine::Ref<ReEngine::Shader> mTetxureShader;
    
    ReEngine::Ref<ReEngine::VertexArray> mVertexArray;
    ReEngine::Ref<ReEngine::VertexArray> mSquareVA;
    ReEngine::Ref<ReEngine::VertexArray> mTetxureVA;
    ReEngine::Ref<ReEngine::Texture2D> mTexture;

    ReEngine::OrthographicCameraController m_CameraController;
    
    glm::vec3 m_SquareColor = { 0.2f, 0.3f, 0.8f };

    
};


