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
    ReEngine::OrthographicCameraController m_CameraController;
};


