#pragma once
#include "ReEngineEditor/SandBox.h"
#include "ReEngineEditor/SandBox.h"
int main(int argc,char** argv)
{
    ReEngine::Log::Init();

    ReEngine::Application::GetInstance().Run();
    
}
