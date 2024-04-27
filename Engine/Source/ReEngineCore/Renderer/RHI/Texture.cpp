#include "Core/PCH.h"
#include "Renderer/RHI/Texture.h"

#include "stb_image.h"
#include "Resource/AssetManager/AssetManager.h"
#include "Renderer/RHI/Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace ReEngine
{
    Ref<TextureData> TextureData::FromeFile(const std::string& FileName, int channel)
    {
        const std::string FilePath = AssetManager::GetFullPath(FileName).generic_string();
        RE_INFO("Load Image : {0}",FilePath);
        
        Ref<TextureData> Image = CreateRef<TextureData>();
        if(stbi_is_hdr(FilePath.c_str()))
        {
            float* pixels = stbi_loadf(FilePath.c_str(), &Image->m_width, &Image->m_height, &Image->m_channels, channel);
            if(pixels) {
                Image->m_pixels.reset(reinterpret_cast<unsigned char*>(pixels));
                Image->m_hdr = true;
            }
        }
        else
        {
            unsigned char* pixels = stbi_load(FilePath.c_str(), &Image->m_width, &Image->m_height, &Image->m_channels, channel);
            if(pixels) {
                Image->m_pixels.reset(pixels);
                Image->m_hdr = false;
            }  
        }

        if(channel > 0)
        {
            Image->m_channels = channel;
        }

        if(!Image->m_pixels)
        {
            RE_ERROR("Failed to load image file:{0}",FileName);
        }

        return Image;
    }

    TextureData::TextureData()
    : m_width(0)
    , m_height(0)
    , m_channels(0)
    , m_hdr(false)
    {
    }

    Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPI::RendererAPIType::None:    RE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
        case RendererAPI::RendererAPIType::OpenGL:  return CreateRef<OpenGLTexture2D>(width, height);
        case RendererAPI::RendererAPIType::Vulkan:  return nullptr;
        case RendererAPI::RendererAPIType::DX11:    return nullptr;
        case RendererAPI::RendererAPIType::DX12:    return nullptr;
        }

        RE_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const std::filesystem::path& path)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPI::RendererAPIType::None:    RE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
        case RendererAPI::RendererAPIType::OpenGL:  return CreateRef<OpenGLTexture2D>(path);
        case RendererAPI::RendererAPIType::Vulkan:  return nullptr;
        case RendererAPI::RendererAPIType::DX11:    return nullptr;
        case RendererAPI::RendererAPIType::DX12:    return nullptr;
        }

        RE_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const std::string& path)
    {
        return Create(AssetManager::GetFullPath(path));
    }

    Ref<Texture3D> Texture3D::Create(uint32_t width, uint32_t height)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPI::RendererAPIType::None:    RE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
        case RendererAPI::RendererAPIType::OpenGL:  return CreateRef<OpenGLTexture3D>(width, height);
        case RendererAPI::RendererAPIType::Vulkan:  return nullptr;
        case RendererAPI::RendererAPIType::DX11:    return nullptr;
        case RendererAPI::RendererAPIType::DX12:    return nullptr;
        }

        RE_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<CubeMapTexture> CubeMapTexture::Create(std::vector<std::string>& paths)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPI::RendererAPIType::None:    RE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
        case RendererAPI::RendererAPIType::OpenGL:  return CreateRef<OpenGLCubeMapTexture>(paths);
        case RendererAPI::RendererAPIType::Vulkan:  return nullptr;
        case RendererAPI::RendererAPIType::DX11:    return nullptr;
        case RendererAPI::RendererAPIType::DX12:    return nullptr;
        }

        RE_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<CubeMapTexture> CubeMapTexture::Create()
    {
        switch (RendererAPI::Current())
        {
        case RendererAPI::RendererAPIType::None:    RE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
        case RendererAPI::RendererAPIType::OpenGL:  return CreateRef<OpenGLCubeMapTexture>();
        case RendererAPI::RendererAPIType::Vulkan:  return nullptr;
        case RendererAPI::RendererAPIType::DX11:    return nullptr;
        case RendererAPI::RendererAPIType::DX12:    return nullptr;
        }

        RE_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<CubeMapTexture> CubeMapTexture::Create(uint32_t width, uint32_t height)
    {
        switch (RendererAPI::Current())
        {
        case RendererAPI::RendererAPIType::None:    RE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
        case RendererAPI::RendererAPIType::OpenGL:  return CreateRef<OpenGLCubeMapTexture>(width, height);
        case RendererAPI::RendererAPIType::Vulkan:  return nullptr;
        case RendererAPI::RendererAPIType::DX11:    return nullptr;
        case RendererAPI::RendererAPIType::DX12:    return nullptr;
        }

        RE_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
}