#pragma once
#include "Core/Core.h"
#include "Core/PCH.h"

#include <string>
#include <filesystem>

namespace ReEngine
{
    //  保存Texture的data
    class TextureData
    {
    public:
        static Ref<TextureData> FromeFile(const std::string& FileName,int channel = 4);

        int width() const { return m_width; }
        int height() const { return m_height; }
        int channels() const { return m_channels; }
        int bytesPerPixel() const { return m_channels * (m_hdr ? sizeof(float) : sizeof(unsigned char)); }
        int pitch() const { return m_width * bytesPerPixel(); }

        bool isHDR() const { return m_hdr; }

        template<typename T>
        const T* pixels() const
        {
            return reinterpret_cast<const T*>(m_pixels.get());
        }

        TextureData();
    private:

        int m_width;
        int m_height;
        int m_channels;
        bool m_hdr;
        Scope<unsigned char> m_pixels;
    };

    
    class Texture
    {
    public:
        virtual ~Texture() = default;
        
        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual uint32_t GetRendererID() const = 0;

        virtual void SetData(void* data,uint32_t size){};
        virtual void Bind(uint32_t slot = 0)const = 0;
        virtual void UnBind() const = 0;

        virtual bool IsLoaded() const{return false;}
        virtual bool operator==(const Texture& other)const = 0;
    };

    class Texture2D : public Texture
    {
    public:
        static Ref<Texture2D> Create(uint32_t width, uint32_t height);
        static Ref<Texture2D> Create(const std::filesystem::path& path);
        static Ref<Texture2D> Create(const std::string& path);
    };

    class Texture3D : public Texture
    {
    public:
        static Ref<Texture3D> Create(uint32_t width, uint32_t height);
    };

    enum class FaceTarget : uint32_t
    {
        Right = 0,
        Left = 1,
        Top = 2,
        Bottom = 3,
        Front = 4,
        Back = 5,
    };

    class CubeMapTexture : public Texture
    {
    public:
        virtual void SetFace(FaceTarget faceIndex, const std::string& path) = 0;
        virtual void GenerateMipmap() = 0;
        static Ref<CubeMapTexture> Create(std::vector<std::string>& paths);
        static Ref<CubeMapTexture> Create();
        static Ref<CubeMapTexture> Create(uint32_t width, uint32_t height);

        virtual void Generate() = 0;

        virtual std::vector<std::string> GetPaths() = 0;
    };
}
