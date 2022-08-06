#pragma once
#include "Core/Core.h"
#include "Core/PCH.h"

namespace ReEngine
{
    enum class FrameBufferTextureFormat
    {
        None = 0,

        // Color 
        RGBA8,
        RED_INTEGER,

        //Depth / stencil
        DEPTH32F_TEX3D,
        DEPTH24STENCIL8,
    };

    struct FrameBufferTextureSpecification
    {
        FrameBufferTextureSpecification() = default;
        FrameBufferTextureSpecification(FrameBufferTextureFormat InFormat) :Format(InFormat) {}

        FrameBufferTextureFormat Format = FrameBufferTextureFormat::None;
    };


    struct FramebufferAttachmentSpecification
    {
        FramebufferAttachmentSpecification() = default;
        FramebufferAttachmentSpecification(std::initializer_list<FrameBufferTextureSpecification> attachments)
            : Attachments(attachments) {}

        std::vector<FrameBufferTextureSpecification> Attachments;
    };

    struct FrameBufferSpecification
    {
        uint32_t Width, Height;
        FramebufferAttachmentSpecification Attachments;
        uint32_t Samples = 1;

        bool SwapChainTaget = false;
    };

    class FrameBuffer
    {
    public:
        virtual ~FrameBuffer() = default;

        virtual void Bind() = 0;
        virtual void BindReadFrameBuffer() = 0;
        virtual void BindDrawFrameBuffer() = 0;
        virtual void UnBind() = 0;

        virtual void Resize(uint32_t width, uint32_t height) = 0;
        virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;


        virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;
        [[nodiscard]] virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;
        [[nodiscard]] virtual uint32_t GetDepthAttachmentRendererID() const = 0;

        [[nodiscard]] virtual const FrameBufferSpecification& GetSpecification() const = 0;

        virtual void FramebufferTexture2D(uint32_t cubemapIndex, uint32_t cubemapID, uint32_t slot = 0) = 0;
        [[nodiscard]] virtual Ref<class Texture3D> GetDepthTex3D() const = 0;

        virtual void BindDepthTex3D(uint32_t slot) = 0;
        virtual void UnBindDepthTex3D(uint32_t slot) = 0;

        static Ref<FrameBuffer> Create(const FrameBufferSpecification& spec);

    };


}