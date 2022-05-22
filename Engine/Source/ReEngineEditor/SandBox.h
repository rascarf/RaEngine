#pragma once

#include "ReEngine.h"

class SandBox :public ReEngine::Application
{
public:
    SandBox();

    ~SandBox();
};

inline ReEngine::Application*ReEngine::CreateApplication()
{
    return new SandBox();
}
