#include "Input/Input.h"
#include "Core/Application.h"

#include <GLFW/glfw3.h>

namespace ReEngine
{
    Input* Input::s_Instance = new WindowsInput;

    bool ReEngine::WindowsInput::IsKeyPressedImpl(int keycode)
    {
        auto window = static_cast<GLFWwindow*>(Application::GetWindow()->GetNativeWindow());
        auto state = glfwGetKey(window, keycode);
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool ReEngine::WindowsInput::IsMouseButtonPressedImpl(int button)
    {
        auto window = static_cast<GLFWwindow*>(Application::GetWindow()->GetNativeWindow());
        auto state = glfwGetMouseButton(window, button);
        return state == GLFW_PRESS;
    }

    bool WindowsInput::IsMouseButtonReleaseImpl(int button)
    {
        auto window = static_cast<GLFWwindow*>(Application::GetWindow()->GetNativeWindow());
        auto state = glfwGetMouseButton(window, button);
        return state == GLFW_RELEASE;
    }
    
    glm::vec2 ReEngine::WindowsInput::GetMousePosImpl()
    {
        auto window = static_cast<GLFWwindow*>(Application::GetWindow()->GetNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return glm::vec2(xpos, ypos);
    }

    float ReEngine::WindowsInput::GetMouseXImpl()
    {
        auto window = static_cast<GLFWwindow*>(Application::GetWindow()->GetNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return (float)xpos;
    }

    float ReEngine::WindowsInput::GetMouseYImpl()
    {
        auto window = static_cast<GLFWwindow*>(Application::GetWindow()->GetNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return (float)ypos;
    }
}


