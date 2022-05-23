#include "SandBox.h"

SandBox::SandBox()
{
    PushOverlay(new ReEngine::ImGuiLayer());
}

SandBox::~SandBox()
{

}
