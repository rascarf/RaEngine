#include "FrameBuffer.h"
#include "Renderer/RHI/Renderer.h"
#include "Platform/OpenGL/OpenGLFrameBuffer.h"

namespace ReEngine
{
    Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& spec)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPI::RendererAPIType::None:    return nullptr;
        case RendererAPI::RendererAPIType::OpenGL:  return CreateRef<OpenGLFrameBuffer>(spec);
        case RendererAPI::RendererAPIType::Vulkan:  return nullptr;
        case RendererAPI::RendererAPIType::DX11:    return nullptr;
        case RendererAPI::RendererAPIType::DX12:    return nullptr;
        }
        return nullptr;
    }
}
