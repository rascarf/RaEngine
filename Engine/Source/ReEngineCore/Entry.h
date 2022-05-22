#pragma once
#include "Event/KeyEvent.h"
#include "Event/ApplicationEvent.h"

extern ReEngine::Application*ReEngine::CreateApplication();

int main(int argc,char** argv)
{
    ReEngine::Log::Init();

    const auto app = ReEngine::CreateApplication();

    app->Run();

    delete app;
}
