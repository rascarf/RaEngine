#pragma once
#include "Renderer/RHI/FrameBuffer.h"

namespace ReEngine 
{
    class OpenGLFrameBuffer : public FrameBuffer 
    {
    public:
        OpenGLFrameBuffer(const FrameBufferSpecification& spec);
        virtual ~OpenGLFrameBuffer();

        void Invalidate();

        virtual void Bind() override;
        virtual void UnBind() override;

        virtual void BindReadFrameBuffer() override;
        virtual void BindDrawFrameBuffer() override;

        virtual void Resize(uint32_t width, uint32_t height) override;
        virtual int ReadPixel(uint32_t attachmentIndex, int x, int y)override;
        virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

        virtual uint32_t GetColorAttachmentRendererID(uint32_t index) const override;
        virtual uint32_t GetDepthAttachmentRendererID() const override;

        virtual const FrameBufferSpecification& GetSpecification()const override{return mSpecification;}
        virtual void FramebufferTexture2D(uint32_t cubemapIndex, uint32_t cubemapID, uint32_t slot) override;
        
        virtual Ref<class Texture3D> GetDepthTex3D() const override;
        virtual void BindDepthTex3D(uint32_t slot) override;
        virtual void UnBindDepthTex3D(uint32_t slot) override;
    private:
        uint32_t mRendererID = 0;
        FrameBufferSpecification mSpecification;
        
        std::vector<uint32_t> mColorAttachments;
        std::vector<FrameBufferTextureSpecification> mColorAttachmentSpecifications;
        
        uint32_t mDepthAttachment = 0;
        FrameBufferTextureSpecification mDepthAttachmentSpecification = FrameBufferTextureFormat::None;
    };    
}

