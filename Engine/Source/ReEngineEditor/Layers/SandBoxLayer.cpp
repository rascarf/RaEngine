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
    ReEngine::Renderer::Init();
}

SandBoxLayer::~SandBoxLayer()
{
    
}

void SandBoxLayer::OnAttach()
{
	ReEngine::FrameBufferSpecification fbSpec;
	fbSpec.Attachments = {ReEngine::FrameBufferTextureFormat::RGBA8,ReEngine::FrameBufferTextureFormat::DEPTH24STENCIL8};
	fbSpec.Width = 1280.0;
	fbSpec.Height = 720.0;
	fbSpec.Samples = 1;
	mFrameBuffer = ReEngine::FrameBuffer::Create(fbSpec);
	mActiveScene = ReEngine::CreateRef<ReEngine::Scene>();
#if 0
	mCameraEntity = mActiveScene->CreateEntity("Camera");
	mCameraEntity.AddComponent<ReEngine::CameraComponnet>(glm::ortho(-16.0f,16.0f,-9.0f,9.0f,-1.0f,1.0f));
	mRenderEntity = mActiveScene->CreateEntity("Sprite");
	mRenderEntity.AddComponent<ReEngine::SpriteRenderComponent>(glm::vec4(0.5,0.5,0.5,1.0));
#endif

	SceneHierarchyPanel.SetContext(mActiveScene);
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
	// Resize
	if (ReEngine::FrameBufferSpecification spec = mFrameBuffer->GetSpecification();
		mViewPortSize.x > 0.0f && mViewPortSize.y > 0.0f &&
		(spec.Width != mViewPortSize.x || spec.Height != mViewPortSize.y))
	{
		mFrameBuffer->Resize((uint32_t)mViewPortSize.x, (uint32_t)mViewPortSize.y);
		mActiveScene->OnViewportResize(mViewPortSize.x,mViewPortSize.y);
	}
	
	mFrameBuffer->Bind();
	ReEngine::RenderCommand::Clear();
	
	mActiveScene->OnUpdate(ts);
	mFrameBuffer->UnBind();
}

void SandBoxLayer::OnUIRender(ReEngine::Timestep ts)
{
	static bool dockingEnabled = true;
	if (dockingEnabled)
	{
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();
	
		if (opt_fullscreen)
			ImGui::PopStyleVar(2);
	
		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
	
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Menu"))
			{
				if (ImGui::MenuItem("Exit")) ReEngine::Application::GetInstance().Shutdown();

				if (ImGui::MenuItem("Serialize"))
				{
					ReEngine::SceneSerializer serializer(mActiveScene);
					serializer.Serialize("Assets/scenes/Example.Re");
				}

				if (ImGui::MenuItem("Deserialize"))
				{
					ReEngine::SceneSerializer serializer(mActiveScene);
					serializer.DeSerialize("Assets/scenes/Example.Re");
				}
				
				ImGui::EndMenu();
			}
			
			ImGui::EndMenuBar();
		}

		// ViewPort
		{
			ImGui::Begin("ViewPort");
			ImVec2 ViewPortPanel = ImGui::GetContentRegionAvail();
			if(ViewPortPanel.x != mViewPortSize.x || ViewPortPanel.y != mViewPortSize.y)
			{
				mViewPortSize.x = ViewPortPanel.x;
				mViewPortSize.y = ViewPortPanel.y;
			}
			
			uint32_t textureID = mFrameBuffer->GetColorAttachmentRendererID();
			ImGui::Image((void*)(intptr_t)textureID,ImVec2{ViewPortPanel.x,ViewPortPanel.y},ImVec2{ 0,1 }, ImVec2{1,0});
			ImGui::End();
		}

		SceneHierarchyPanel.OnImGuiRender();
		
		ImGui::End();
	}
}

