#include "SandBox.h"

#include "Layers/StencilLayer.h"
#include "Layers/InputAttachMent.h"
#include "Layers/MSAALayer.h"
#include "Layers/SandBoxLayer.h"
#include "Layers/AnimationLayers/AnimationLayer.h"
#include "Layers/AnimationLayers/AnimationTextureLayer.h"
#include "Layers/ComputeLayers/ComputeLayer.h"
#include "Layers/RayTracing/SimplePathTracing.h"
#include "Layers/RenderPath/TileBasedForwardLayer.h"
#include "Layers/FrameGraphs/FrameGraphTest/TemplateLayer.h"
#include "Layers/GPUDriven/GPUDrivenLayer.h"

namespace ReEngine
{
    void AppInit(Application& app)
    {
        // const auto Layer = CreateRef<RTLayer>();
        // const auto Layer = CreateRef<SimplePathTracing>();
        const auto Layer = CreateRef<GPUDrivenLayer>();
        
        app.PreInit();
        app.Init();
        app.PushLayer(Layer);

        // app.PushLayer(CreateRef<SimplePathTracing>());
        // app.PushLayer(CreateRef<TileBasedForwardLayer>());
        // app.PushLayer(CreateRef<MSAALayer>());
        // app.PushLayer(CreateRef<AnimationTextureLayer>());
        // app.PushLayer(CreateRef<AnimationLayer>());
        // app.PushLayer(CreateRef<InputAttachment>());
        // app.PushLayer(CreateRef<SandBoxLayer>());
        // app.PushLayer(CreateRef<ComputeLayer>());
        // app.PushLayer(CreateRef<FrameGraphTemplateLayer>());
    }
}
