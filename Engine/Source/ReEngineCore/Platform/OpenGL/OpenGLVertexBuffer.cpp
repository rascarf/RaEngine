#include "OpenGLVertexBuffer.h"
#include "glad/glad.h"

namespace ReEngine
{
    static GLenum OpenGLUsage(VertexBufferUsage usage)
    {
        switch (usage)
        {
        case VertexBufferUsage::Dynamic : return GL_STATIC_DRAW;
        case VertexBufferUsage::Static  : return GL_DYNAMIC_DRAW;

            RE_CORE_ERROR("Unknow Vertex Buffer Usage");
            return 0;
        }
    }
    
    OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size, VertexBufferUsage usage):mUsage(usage)
    {
        glCreateBuffers(1,&mRendererID);
        glBindBuffer(GL_ARRAY_BUFFER, mRendererID);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, OpenGLUsage(usage));
        
    }

    OpenGLVertexBuffer::OpenGLVertexBuffer(void* vertices, uint32_t size, VertexBufferUsage usage):mUsage(usage)
    {
        glCreateBuffers(1,&mRendererID);
        glBindBuffer(GL_ARRAY_BUFFER, mRendererID);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, OpenGLUsage(usage));
    }

    OpenGLVertexBuffer::~OpenGLVertexBuffer()
    {
        glDeleteBuffers(1,&mRendererID);
    }

    void OpenGLVertexBuffer::Bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER,mRendererID);
    }

    void OpenGLVertexBuffer::Unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void OpenGLVertexBuffer::SetData(const void* data, uint32_t size)
    {
        glBindBuffer(GL_ARRAY_BUFFER, mRendererID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
    }
}
