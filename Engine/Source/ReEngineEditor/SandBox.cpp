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

namespace ReEngine
{
    void AppInit(Application& app)
    {
        app.Init();

        app.PushLayer(CreateRef<SimplePathTracing>());
        // app.PushLayer(CreateRef<TileBasedForwardLayer>());
        // app.PushLayer(CreateRef<MSAALayer>());
        // app.PushLayer(CreateRef<AnimationTextureLayer>());
        // app.PushLayer(CreateRef<AnimationLayer>());
        // app.PushLayer(CreateRef<RTLayer>());
        // app.PushLayer(CreateRef<InputAttachment>());
        // app.PushLayer(CreateRef<SandBoxLayer>());
        // app.PushLayer(CreateRef<ComputeLayer>());
    }
}
