#pragma once

#include "Renderer/IndexBuffer.h"

namespace ReEngine
{
    class OpenGLIndexBuffer : public IndexBuffer
    {
    public:
        OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
        OpenGLIndexBuffer(uint32_t count);
        virtual ~OpenGLIndexBuffer() override;

        virtual void Bind() const override;
        virtual void Unbind() const override;
        virtual void SetData(const void* data, uint32_t count) override;

        [[nodiscard]]virtual uint32_t GetCount() const override { return mCount; }
    private:
        uint32_t mRendererID;
        uint32_t mCount;
    };
    
}
