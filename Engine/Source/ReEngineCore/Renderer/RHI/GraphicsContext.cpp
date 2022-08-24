#include "Renderer/RHI/GraphicsContext.h"
#include "Platform/OpenGL/OpenGLContext.h"
#include "Renderer/RHI/Renderer.h"

namespace ReEngine
{
    Ref<GraphicsContext> GraphicsContext::Create(void* Window)
    {
        switch (RendererAPI::Current())
        {
            case RendererAPI::RendererAPIType::OpenGL:  return CreateRef<OpenGLContext>(static_cast<GLFWwindow*>(Window));
            case RendererAPI::RendererAPIType::DX12:    return nullptr;
        }

        return nullptr;
    }
}


