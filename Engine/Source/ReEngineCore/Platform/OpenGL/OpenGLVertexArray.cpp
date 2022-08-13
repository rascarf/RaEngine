#include "OpenGLVertexArray.h"
#include <glad/glad.h>
namespace ReEngine
{
    static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
    {
        switch (type)
        {
        case ReEngine::ShaderDataType::Float:     return GL_FLOAT;
        case ReEngine::ShaderDataType::Float2:	 return GL_FLOAT;
        case ReEngine::ShaderDataType::Float3:	 return GL_FLOAT;
        case ReEngine::ShaderDataType::Float4:	 return GL_FLOAT;
        case ReEngine::ShaderDataType::Mat3:		 return GL_FLOAT;
        case ReEngine::ShaderDataType::Mat4:		 return GL_FLOAT;
        case ReEngine::ShaderDataType::Int:		 return GL_INT;
        case ReEngine::ShaderDataType::Int2:		 return GL_INT;
        case ReEngine::ShaderDataType::Int3:		 return GL_INT;
        case ReEngine::ShaderDataType::Int4:		 return GL_INT;
        case ReEngine::ShaderDataType::Bool:		 return GL_BOOL;
        }
        
        return 0;
    }
    OpenGLVertexArray::OpenGLVertexArray()
    {
        glCreateVertexArrays(1, &mRendererID);
    }

    OpenGLVertexArray::~OpenGLVertexArray()
    {
        glDeleteVertexArrays(1, &mRendererID);
    }

    void OpenGLVertexArray::Bind() const
    {
        glBindVertexArray(mRendererID);
    }

    void OpenGLVertexArray::Unbind() const
    {
        glBindVertexArray(0);
    }

    void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
    {
        glBindVertexArray(mRendererID);
        vertexBuffer->Bind();

        const auto& layout = vertexBuffer->GetLayout();
        for (const auto& element : layout)
        {
            switch (element.Type)
            {
            case ShaderDataType::Float:
        case ShaderDataType::Float2:
        case ShaderDataType::Float3:
        case ShaderDataType::Float4:
                {
                    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,3 * sizeof(float),nullptr);
                    glEnableVertexAttribArray(mVertexBufferIndex);
                    glVertexAttribPointer(mVertexBufferIndex,
                        element.GetComponentCount(),
                        ShaderDataTypeToOpenGLBaseType(element.Type),
                        element.Normalized ? GL_TRUE : GL_FALSE,
                        layout.GetStride(),
                        (const void*)element.Offset);
                    mVertexBufferIndex++;
                    break;
                }
            case ShaderDataType::Int:
            case ShaderDataType::Int2:
            case ShaderDataType::Int3:
            case ShaderDataType::Int4:
            case ShaderDataType::Bool:
                {
                    glEnableVertexAttribArray(mVertexBufferIndex);
                    glVertexAttribIPointer(mVertexBufferIndex,
                        element.GetComponentCount(),
                        ShaderDataTypeToOpenGLBaseType(element.Type),
                        layout.GetStride(),
                        (const void*)element.Offset);
                    mVertexBufferIndex++;
                    break;
                }
            case ShaderDataType::Mat3:
            case ShaderDataType::Mat4:
                {
                    uint8_t count = element.GetComponentCount();
                    for (uint8_t i = 0; i < count; i++)
                    {
                        glEnableVertexAttribArray(mVertexBufferIndex);
                        glVertexAttribPointer(mVertexBufferIndex,
                            count,
                            ShaderDataTypeToOpenGLBaseType(element.Type),
                            element.Normalized ? GL_TRUE : GL_FALSE,
                            layout.GetStride(),
                            (const void*)(element.Offset + sizeof(float) * count * i));
                        glVertexAttribDivisor(mVertexBufferIndex, 1);
                        mVertexBufferIndex++;
                    }
                    break;
                }
            default:
                {
                }
            }
        }
        
        mVertexBuffers.push_back(vertexBuffer);
    }

    void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
    {
        glBindVertexArray(mRendererID);
        indexBuffer->Bind();

        mIndexBuffer = indexBuffer;
    }
}
