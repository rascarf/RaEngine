#include "Shader.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Renderer/RHI/Renderer.h"

namespace ReEngine
{
    Ref<Shader> Shader::Create(const std::filesystem::path& filepath)
    {
        return Create(filepath.string());
    }

    Ref<Shader> Shader::Create(const std::string& filepath)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPI::RendererAPIType::None:     return nullptr;
        case RendererAPI::RendererAPIType::OpenGL:  return std::make_shared<OpenGLShader>(filepath);
        case RendererAPI::RendererAPIType::Vulkan:  return nullptr;
        case RendererAPI::RendererAPIType::DX11:    return nullptr;
        case RendererAPI::RendererAPIType::DX12:    return nullptr;
        }
        
        return nullptr;
    }

    Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPI::RendererAPIType::None:     return nullptr;
        case RendererAPI::RendererAPIType::OpenGL:  return std::make_shared<OpenGLShader>(name, vertexSrc, fragmentSrc);
        case RendererAPI::RendererAPIType::Vulkan:  return nullptr;
        case RendererAPI::RendererAPIType::DX11:    return nullptr;
        case RendererAPI::RendererAPIType::DX12:    return nullptr;
        }
        
        return nullptr;
    }
    

    ShaderUniform::ShaderUniform(const std::string& name, ShaderUniformType type, uint32_t size, uint32_t offset)
        : mName(name), mType(type), mSize(size), mOffset(offset)
    {
    }

    const std::string& ShaderUniform::UniformTypeToString(ShaderUniformType type)
    {
        if (type == ShaderUniformType::Bool)
        {
            return "Boolean";
        }
        else if (type == ShaderUniformType::Int)
        {
            return "Int";
        }
        else if (type == ShaderUniformType::Float)
        {
            return "Float";
        }

        return "None";
    }
}
