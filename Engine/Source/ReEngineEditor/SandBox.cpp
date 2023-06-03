#include "SandBox.h"

#include "Layers/StencilLayer.h"
#include "Layers/InputAttachMent.h"
#include "Layers/SandBoxLayer.h"
#include "Layers/AnimationLayer.h"
namespace ReEngine
{
    void AppInit(Application& app)
    {
        app.Init();

        app.PushLayer(CreateRef<AnimationLayer>());
        // app.PushLayer(CreateRef<RTLayer>());
        // app.PushLayer(CreateRef<InputAttachment>());
        // app.PushLayer(CreateRef<SandBoxLayer>());
    }
}
