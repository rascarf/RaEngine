#pragma once

#include "Platform/Vulkan/RayTracing/Scene/SimpleglTFScene.h"
#include "ReEngineEditor/Layers/GraphicalLayer.h"
#include "ReEngineEditor/Layers/TemplateLayer.h"

class GPUDrivenLayer : public TemplateLayer
{
public:
    virtual void OnNativeLoadAsset() override;
    virtual void OnNativeUpdate(Timestep ts) override;
    virtual void OnNativeDeInit() override;
};
