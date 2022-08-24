#include "VertexArray.h"
#include "Renderer/RHI/Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
namespace ReEngine
{
    Ref<VertexArray> VertexArray::Create()
    {
        switch (RendererAPI::Current())
        {
        case RendererAPI::RendererAPIType::None:     return nullptr;
        case RendererAPI::RendererAPIType::OpenGL:  return std::make_shared<OpenGLVertexArray>();
        case RendererAPI::RendererAPIType::Vulkan:  return nullptr;
        case RendererAPI::RendererAPIType::DX11:    return nullptr;
        case RendererAPI::RendererAPIType::DX12:    return nullptr;
        }
        
        return nullptr;
    }
}
