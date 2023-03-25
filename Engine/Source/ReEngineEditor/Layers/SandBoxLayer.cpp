#include "SandBoxLayer.h"

#include "imgui.h"
#include "ReEngine.h"
#include "FrameWork/Scene.h"
#include "FrameWork/Component/CameraComponent.h"
#include "FrameWork/Component/SpriteRenderComponent.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "FrameWork/SceneSerialization.h"
#include "Renderer/Renderer2D.h"


SandBoxLayer::SandBoxLayer():Layer("SandBoxLayer")
{
   
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
	if(e->GetEventType() == ReEngine::EventType::WindowResize)
	{
		ReEngine::Ref<ReEngine::WindowResizeEvent> re = std::dynamic_pointer_cast<ReEngine::WindowResizeEvent>(e);
	}
	else if (e->GetEventType() == ReEngine::EventType::KeyPressed)
	{
		
	}
}

void SandBoxLayer::OnUpdate(ReEngine::Timestep ts)
{
	
}

void SandBoxLayer::OnUIRender(ReEngine::Timestep ts)
{
	
}

