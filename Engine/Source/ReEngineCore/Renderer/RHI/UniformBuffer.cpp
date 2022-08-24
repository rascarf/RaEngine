#include "UniformBuffer.h"
#include "Renderer/RHI/Renderer.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"
namespace ReEngine
{
    Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPI::RendererAPIType::None:     return nullptr;
        case RendererAPI::RendererAPIType::OpenGL:  return CreateRef<OpenGLUniformBuffer>(size, binding);
        case RendererAPI::RendererAPIType::Vulkan:  return nullptr;
        case RendererAPI::RendererAPIType::DX11:    return nullptr;
        case RendererAPI::RendererAPIType::DX12:    return nullptr;
        }
        
        return nullptr;
    }
}
