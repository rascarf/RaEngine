#include "Log/Log.h"
#include "ImGuiLayer.h"
#include "Event/EventDispatcher.h"
#include "Core/Application.h"

#include <imgui/imgui.h>
#include <GLFW/glfw3.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>



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

void ReEngine::ImGuiLayer::OnEvent(std::shared_ptr<Event> e)
{
    Layer::OnEvent(e);

    EventDispatcher dispatcher(e);

    dispatcher.DispatchEvent<MouseButtonPressedEvent>(RE_BIND_EVENT_FN(MouseButtonPressedEvent, ImGuiLayer::OnMouseButtonPressedEvent));

    dispatcher.DispatchEvent<MouseButtonReleasedEvent>(RE_BIND_EVENT_FN(MouseButtonReleasedEvent, ImGuiLayer::OnMouseButtonReleasedEvent));

    dispatcher.DispatchEvent<MouseMoveEvent>(RE_BIND_EVENT_FN(MouseMoveEvent, ImGuiLayer::OnMouseMovedEvent));
  
    dispatcher.DispatchEvent<MouseScrollEvent>(RE_BIND_EVENT_FN(MouseScrollEvent,ImGuiLayer::OnMouseScrolledEvent));
   
    dispatcher.DispatchEvent<KeyPressedEvent>(RE_BIND_EVENT_FN(KeyPressedEvent,ImGuiLayer::OnKeyPressedEvent));

    dispatcher.DispatchEvent<KeyReleasedEvent>(RE_BIND_EVENT_FN(KeyReleasedEvent,ImGuiLayer::OnKeyReleasedEvent));

    dispatcher.DispatchEvent<KeyTypedEvent>(RE_BIND_EVENT_FN(KeyTypedEvent, ImGuiLayer::OnKeyTypedEvent));

    dispatcher.DispatchEvent<WindowResizeEvent>(RE_BIND_EVENT_FN(WindowResizeEvent,ImGuiLayer::OnWindowResizeEvent));
}

void ReEngine::ImGuiLayer::OnAttach()
{
    Layer::OnAttach();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;


    ImGui_ImplOpenGL3_Init("#version 410");
}

void ReEngine::ImGuiLayer::OnDetach()
{
    Layer::OnDetach();
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
