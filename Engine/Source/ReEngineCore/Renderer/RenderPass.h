#pragma once
#include "FrameBuffer.h"

namespace ReEngine
{
    struct RenderPassSpecification
    {
        Ref<FrameBuffer> TargetFramebuffer = nullptr;
        std::string DebugName = "";
    };

    class RenderPass
    {
    public:
        RenderPass(RenderPassSpecification Spec) : mSpecification(Spec) {};
        virtual ~RenderPass() = default;

    public:
        RenderPassSpecification& GetSpecification() { return mSpecification; }
        uint32_t ExcuteAndReturnFinalTex();
       
    private:
        RenderPassSpecification mSpecification;
    };
}
