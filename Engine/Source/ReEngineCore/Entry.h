#pragma once
#include "core/Application.h"
namespace ReEngine
{
    extern void AppInit(Application& app); //交给Editor去做操作
}

int main(int argc,char** argv)
{
    ReEngine::Log::Init();

    ReEngine::AppInit(ReEngine::Application::GetInstance());

    ReEngine::Application::GetInstance().Run();

    ReEngine::Application::GetInstance().Shutdown();
}


