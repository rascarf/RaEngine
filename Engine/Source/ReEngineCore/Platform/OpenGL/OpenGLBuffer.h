#pragma once

#include "Renderer/Buffer.h"

namespace ReEngine
{
    class OpenGLVertexBuffer : public VertexBuffer
    {
    public:
        OpenGLVertexBuffer(float* vertices, uint32_t size);
        virtual ~OpenGLVertexBuffer();

        virtual void Bind()const;
        virtual void UnBind() const;

    private:
        uint32_t m_RendererID;

    };

    class OpenGLIndexBuffer : puiblic IndexBuffer
    {

    };
}