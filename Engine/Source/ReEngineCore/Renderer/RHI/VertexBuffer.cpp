#include "VertexBuffer.h"
#include "Renderer/RHI/Renderer.h"
#include "Platform/OpenGL/OpenGLVertexBuffer.h"

namespace ReEngine
{
    Ref<VertexBuffer> VertexBuffer::Create(uint32_t size, VertexBufferUsage usage)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPI::RendererAPIType::None:     return nullptr;
        case RendererAPI::RendererAPIType::OpenGL:  return CreateRef<OpenGLVertexBuffer>(size, usage);
        case RendererAPI::RendererAPIType::Vulkan:  return nullptr;
        case RendererAPI::RendererAPIType::DX11:    return nullptr;
        case RendererAPI::RendererAPIType::DX12:    return nullptr;
        }

        return nullptr;
    }

    Ref<VertexBuffer> VertexBuffer::Create(void* vertices, uint32_t size, VertexBufferUsage usage)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPI::RendererAPIType::None:     return nullptr;
        case RendererAPI::RendererAPIType::OpenGL:  return CreateRef<OpenGLVertexBuffer>(vertices, size, usage);
        case RendererAPI::RendererAPIType::Vulkan:  return nullptr;
        case RendererAPI::RendererAPIType::DX11:    return nullptr;
        case RendererAPI::RendererAPIType::DX12:    return nullptr;
        }

        return nullptr;
    }
}
