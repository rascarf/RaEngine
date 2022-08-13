#include "Log/Log.h"
#include "ImGuiLayer.h"
#include "Event/EventDispatcher.h"
#include "Core/Application.h"

#include <imgui.h>

#include <GLFW/glfw3.h>
#include <imgui_spectrum.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>


ReEngine::ImGuiLayer::ImGuiLayer():Layer("ImGuiLayer")
{

}

ReEngine::ImGuiLayer::~ImGuiLayer()
{
}


void ReEngine::ImGuiLayer::OnEvent(std::shared_ptr<Event> e)
{
    EventDispatcher dispatcher(e);
    //
    // dispatcher.DispatchEvent<MouseButtonPressedEvent>(RE_BIND_EVENT_FN(MouseButtonPressedEvent, ImGuiLayer::OnMouseButtonPressedEvent));
    //
    // dispatcher.DispatchEvent<MouseButtonReleasedEvent>(RE_BIND_EVENT_FN(MouseButtonReleasedEvent, ImGuiLayer::OnMouseButtonReleasedEvent));
    //
    // dispatcher.DispatchEvent<MouseMoveEvent>(RE_BIND_EVENT_FN(MouseMoveEvent, ImGuiLayer::OnMouseMovedEvent));
    //
    // dispatcher.DispatchEvent<MouseScrollEvent>(RE_BIND_EVENT_FN(MouseScrollEvent,ImGuiLayer::OnMouseScrolledEvent));
    //
    // dispatcher.DispatchEvent<KeyPressedEvent>(RE_BIND_EVENT_FN(KeyPressedEvent,ImGuiLayer::OnKeyPressedEvent));
    //
    // dispatcher.DispatchEvent<KeyReleasedEvent>(RE_BIND_EVENT_FN(KeyReleasedEvent,ImGuiLayer::OnKeyReleasedEvent));
    //
    // dispatcher.DispatchEvent<KeyTypedEvent>(RE_BIND_EVENT_FN(KeyTypedEvent, ImGuiLayer::OnKeyTypedEvent));
    //
    dispatcher.DispatchEvent<WindowResizeEvent>(RE_BIND_EVENT_FN(WindowResizeEvent,ImGuiLayer::OnWindowResizeEvent));
}

void ReEngine::ImGuiLayer::OnUpdate()
{
    Layer::OnUpdate();
}

void ReEngine::ImGuiLayer::Begin()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ReEngine::ImGuiLayer::End()
{
    ImGuiIO& io = ImGui::GetIO();
    Application& app = Application::Get();
    io.DisplaySize = ImVec2(app.GetWindow()->GetWindowWidth(), app.GetWindow()->GetWindowHeight());

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

}

void ReEngine::ImGuiLayer::OnUIRender()
{
    static bool show = true;
    ImGui::ShowDemoWindow(&show);
}

void ReEngine::ImGuiLayer::OnAttach()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // ImGui::StyleColorsDark();
    ImGui::Spectrum::StyleColorsSpectrum();

    ImGuiIO& io = ImGui::GetIO();

    io.Fonts->AddFontDefault();
    ImFont* font = io.Fonts->AddFontFromFileTTF("./Resources/Fonts/SourceSans3-Regular.ttf", 24.0f);
    if (font) io.FontDefault = font;

    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGuiStyle& style = ImGui::GetStyle();
    if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    Application& app = Application::Get();
    auto window = static_cast<GLFWwindow*>(app.GetWindow()->GetNativeWindow());
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}

void ReEngine::ImGuiLayer::OnDetach()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

bool ReEngine::ImGuiLayer::OnMouseButtonPressedEvent(std::shared_ptr<MouseButtonPressedEvent> e)
{
    RE_CORE_INFO("Press");
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[e->GetMouseButton()] = true;

    return false;
}

bool ReEngine::ImGuiLayer::OnMouseButtonReleasedEvent(std::shared_ptr<MouseButtonReleasedEvent> e)
{
    RE_CORE_INFO("Release");
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[e->GetMouseButton()] = false;

    return false;
}

bool ReEngine::ImGuiLayer::OnMouseMovedEvent(std::shared_ptr<MouseMoveEvent> e)
{
    // RE_CORE_INFO("Move");
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos = ImVec2(e->GetX(), e->GetY());

    return false;
}

bool ReEngine::ImGuiLayer::OnMouseScrolledEvent(std::shared_ptr<MouseScrollEvent> e)
{
    // RE_CORE_INFO("Scroll");
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += e->GetXOffset();
    io.MouseWheel += e->GetYOffset();

    return false;
}

bool ReEngine::ImGuiLayer::OnKeyPressedEvent(std::shared_ptr<KeyPressedEvent> e)
{
    RE_CORE_INFO("keyPressed");
    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[e->getKeyCode()] = true;

    // TODO: Key modifiers
    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

    return false;
}

bool ReEngine::ImGuiLayer::OnKeyReleasedEvent(std::shared_ptr<KeyReleasedEvent> e)
{
    RE_CORE_INFO("keyRelease");
    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[e->getKeyCode()] = false;

    return false;
}

bool ReEngine::ImGuiLayer::OnKeyTypedEvent(std::shared_ptr<KeyTypedEvent> e)
{
    ImGuiIO& io = ImGui::GetIO();
    int keycode = e->getKeyCode();

    if (keycode > 0 && keycode < 0x10000)
        io.AddInputCharacter((unsigned short)keycode);

    return false;

}

bool ReEngine::ImGuiLayer::OnWindowResizeEvent(std::shared_ptr<WindowResizeEvent> e)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(e->GetWidth(), e->GetHeight());
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
    glViewport(0, 0, e->GetWidth(), e->GetHeight());

    return false;
}
