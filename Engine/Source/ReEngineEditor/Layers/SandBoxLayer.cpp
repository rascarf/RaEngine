#include "SandBoxLayer.h"

#include "imgui.h"
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
	ReEngine::FrameBufferSpecification fbSpec;
	fbSpec.Attachments = {ReEngine::FrameBufferTextureFormat::RGBA8,ReEngine::FrameBufferTextureFormat::DEPTH24STENCIL8};
	fbSpec.Width = 1280.0;
	fbSpec.Height = 720.0;
	fbSpec.Samples = 1;
	mFrameBuffer = ReEngine::FrameBuffer::Create(fbSpec);
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
	{
		mFrameBuffer->Bind();
		// glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		ReEngine::RenderCommand::Clear();
		ReEngine::Renderer2D::BeginScene(m_CameraController.GetCamera());
		ReEngine::Renderer2D::DrawQuad(glm::vec3(0,0,1),glm::vec2(1,1),glm::vec4(0.5,1.0,1.0,1.0));
		ReEngine::Renderer2D::EndScene();
		// glBindFramebuffer(GL_FRAMEBUFFER, 0);
		mFrameBuffer->UnBind();
	}
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
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit")) ReEngine::Application::GetInstance().Shutdown();
				ImGui::EndMenu();
			}
			
			ImGui::EndMenuBar();
		}

		ImGui::Begin("Scene");
		uint32_t textureID = mFrameBuffer->GetColorAttachmentRendererID();
		ImGui::Image((void*)(intptr_t)textureID,ImVec2{1280,720},ImVec2{ 0,1 }, ImVec2{1,0});
		ImGui::End();
		
		ImGui::End();
	}
}

