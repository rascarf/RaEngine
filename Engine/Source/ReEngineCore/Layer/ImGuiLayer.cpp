#include "ImGuiLayer.h"
#include <imgui/imgui.h>

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "Core/Application.h"

ReEngine::ImGuiLayer::ImGuiLayer():Layer("ImGuiLayer")
{

}

ReEngine::ImGuiLayer::~ImGuiLayer()
{
}

void ReEngine::ImGuiLayer::OnUpdate()
{
    Layer::OnUpdate();

    ImGuiIO& io = ImGui::GetIO();
    Application& app = Application::Get();
    io.DisplaySize = ImVec2(app.GetWindow()->GetWindowWidth(), app.GetWindow()->GetWindowHeight());

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    static bool show = true;
    ImGui::ShowDemoWindow(&show);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ReEngine::ImGuiLayer::OnEvent(Event* e)
{
    Layer::OnEvent(e);
}

void ReEngine::ImGuiLayer::OnAttach()
{
    Layer::OnAttach();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    ImGui_ImplOpenGL3_Init("#version 410");


}

void ReEngine::ImGuiLayer::OnDetach()
{
    Layer::OnDetach();
}
