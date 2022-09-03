#include "SandBoxLayer.h"
#include "ReEngine.h"
#include "Renderer/Renderer2D.h"

SandBoxLayer::SandBoxLayer():Layer("SandBoxLayer"),m_CameraController(1280.0f / 720.0f)
{
    ReEngine::Renderer::Init();
}

SandBoxLayer::~SandBoxLayer()
{
    
}

void SandBoxLayer::OnAttach()
{
	
}

void SandBoxLayer::OnDetach()
{
    
}

void SandBoxLayer::OnEvent(std::shared_ptr<ReEngine::Event> e)
{
    m_CameraController.OnEvent(e);

	if(e->GetEventType() == ReEngine::EventType::WindowResize)
	{
		ReEngine::Ref<ReEngine::WindowResizeEvent> re = std::dynamic_pointer_cast<ReEngine::WindowResizeEvent>(e);
		m_CameraController.SetZoomLevel(re->GetHeight() / 720.0f);
	}
	else if (e->GetEventType() == ReEngine::EventType::KeyPressed)
	{
		RE_INFO("{0},{1},{2}",m_CameraController.GetCamera().GetPosition().x,m_CameraController.GetCamera().GetPosition().y,m_CameraController.GetCamera().GetPosition().z);
	}
}

void SandBoxLayer::OnUpdate(ReEngine::Timestep ts)
{
	m_CameraController.OnUpdate(ts);
	ReEngine::RenderCommand::Clear();
	ReEngine::Renderer2D::BeginScene(m_CameraController.GetCamera());

	ReEngine::Renderer2D::DrawQuad(glm::vec3(0,0,1),glm::vec2(1,1),glm::vec4(0.5,1.0,1.0,1.0));
	
	ReEngine::Renderer::EndScene();
}
