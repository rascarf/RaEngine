#pragma once
#include "Core/Core.h"
#include "Core/PCH.h"
namespace ReEngine
{
    enum class ShaderDataType
    {
        None = 0,
        Float,
        Float2,
        Float3,
        Float4,
        Mat3,
        Mat4,
        Int,
        Int2 ,
        Int3,
        Int4,
        Bool
    };

    static uint32_t ShderDataTypeSize(ShaderDataType type)
    {
        switch (type)
        {
        case ReEngine::ShaderDataType::Float:    return 4;
        case ReEngine::ShaderDataType::Float2:   return 4 * 2;
        case ReEngine::ShaderDataType::Float3:   return 4 * 3;
        case ReEngine::ShaderDataType::Float4:   return 4 * 4;
        case ReEngine::ShaderDataType::Mat3:     return 4 * 3 * 3;
        case ReEngine::ShaderDataType::Mat4:     return 4 * 4 * 4;
        case ReEngine::ShaderDataType::Int:      return 4;
        case ReEngine::ShaderDataType::Int2:     return 4 * 2;
        case ReEngine::ShaderDataType::Int3:     return 4 * 3;
        case ReEngine::ShaderDataType::Int4:     return 4 * 4;
        case ReEngine::ShaderDataType::Bool:     return 1;
        }
        
        return 0;
    }

    struct BufferElement
    {
        std::string Name;
        ShaderDataType Type;
        uint32_t Size;
        uint32_t Offset;
        bool Normalized;

        BufferElement() = default;

        BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
            : Name(name), Type(type), Size(ShderDataTypeSize(type)), Offset(0), Normalized(normalized)
        {
        }

        uint32_t GetComponentCount() const
        {
            switch (Type)
            {
            case ReEngine::ShaderDataType::Float:    return 1;
            case ReEngine::ShaderDataType::Float2:   return 2;
            case ReEngine::ShaderDataType::Float3:   return 3;
            case ReEngine::ShaderDataType::Float4:   return 4;
            case ReEngine::ShaderDataType::Mat3:     return 3 * 3;
            case ReEngine::ShaderDataType::Mat4:     return 4 * 4;
            case ReEngine::ShaderDataType::Int:      return 1;
            case ReEngine::ShaderDataType::Int2:     return 2;
            case ReEngine::ShaderDataType::Int3:     return 3;
            case ReEngine::ShaderDataType::Int4:     return 4;
            case ReEngine::ShaderDataType::Bool:     return 1;
            }
            
            return 0;
        }
    };

    class BufferLayout
    {
    public:
        BufferLayout() = default;

        BufferLayout(const std::initializer_list<BufferElement>& elements)
            : mElements(elements)
        {
            CalculateOffsetsAndStride();
        }

        [[nodiscard]] inline uint32_t GetStride() const { return mStride; }
        [[nodiscard]] inline const std::vector<BufferElement>& GetElements() const { return mElements; }

        std::vector<BufferElement>::iterator begin() { return mElements.begin(); }
        std::vector<BufferElement>::iterator end() { return mElements.end(); }
        std::vector<BufferElement>::const_iterator begin() const { return mElements.begin(); }
        std::vector<BufferElement>::const_iterator end() const { return mElements.end(); }

    private:

        void CalculateOffsetsAndStride()
        {
            uint32_t offset = 0;
            mStride = 0;
            for (auto& element : mElements)
            {
                element.Offset = offset;
                offset += element.Size;
                mStride += element.Size;
            }
        }

    private:
        std::vector<BufferElement> mElements;
        uint32_t mStride = 0;
    };

    enum class VertexBufferUsage
    {
        None = 0, Static = 1, Dynamic = 2
    };

    class VertexBuffer
    {
    public:
        virtual ~VertexBuffer() = default;

        virtual void Bind()const = 0;
        virtual void Unbind()const = 0;

        virtual void SetData(const void* data, uint32_t size) = 0;

        virtual const BufferLayout& GetLayout() const = 0;
        virtual void SetLayout(const BufferLayout& layout) = 0;


        static Ref<VertexBuffer> Create(uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Dynamic);
        static Ref<VertexBuffer> Create(void* vertices, uint32_t size, VertexBufferUsage usage = VertexBufferUsage::Static);
    };

}

