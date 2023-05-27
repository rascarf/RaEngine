#include "SandBox.h"

#include "Layers/StencilLayer.h"
#include "Layers/InputAttachMent.h"
#include "Layers/SandBoxLayer.h"
namespace ReEngine
{
    void AppInit(Application& app)
    {
        app.Init();

        app.PushLayer(CreateRef<RTLayer>());
        // app.PushLayer(CreateRef<InputAttachment>());
        // app.PushLayer(CreateRef<SandBoxLayer>());
    }
}
