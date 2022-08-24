#pragma once
#include "RendererAPI.h"

namespace ReEngine
{
    class RenderCommand
    {
    public:
        static void Init()
        {
            s_RendererAPI->Init();
        }

        static void SetViewport(uint32_t x, uint32_t y,uint32_t width,uint32_t height)
        {
            s_RendererAPI->SetViewport(x, y, width, height);
        }

        static void SetClearColor(const glm::vec4& color)
        {
            s_RendererAPI->SetClearColor(color);
        }

        static void Clear()
        {
            s_RendererAPI->Clear();
        }

        static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t IndexCount = 0)
        {
            s_RendererAPI->DrawIndexed(vertexArray, IndexCount);
        }

        static void DrawLines(const Ref<VertexArray>& vertexArray,uint32_t vertexCount)
        {
            s_RendererAPI->DrawLines(vertexArray, vertexCount);
        }

        static void SetLineWidth(float width)
        {
            s_RendererAPI->SetLineWidth(width);
        }

		static void DepthMask(bool maskFlag)
		{
			s_RendererAPI->DepthMask(maskFlag);
		}

		//Open/Close Depth Test
		static void DepthTest(bool enable)
		{
			s_RendererAPI->DepthTest(enable);
		}

		static void DepthFunc(DepthComp comp)
		{
			s_RendererAPI->DepthFunc(comp);
		}

		//blend
		static void Blend(int32_t Bit)
		{
			s_RendererAPI->Blend(Bit);
		}

		static void BindTexture(int32_t slot, uint32_t textureID)
		{
			s_RendererAPI->BindTexture(slot, textureID);
		}

		//stencil
		static void StencilTest(int32_t Bit)
		{
			s_RendererAPI->StencilTest(Bit);
		}

		static void SetStencilFunc(StencilFunc stencilFunc, int32_t ref, int32_t mask)
		{
			s_RendererAPI->SetStencilFunc(stencilFunc, ref, mask);
		}

		static void SetFrontOrBackStencilOp(int32_t FrontOrBack, StencilOp stencilFail, StencilOp depthFail, StencilOp depthSuccess)
		{
			s_RendererAPI->SetFrontOrBackStencilOp(FrontOrBack, stencilFail, depthFail, depthSuccess);
		}

		static void ClearStencil()
		{
			s_RendererAPI->ClearStencil();
		}

		static void StencilMask(uint32_t mask)
		{
			s_RendererAPI->StencilMask(mask);
		}

		//cull
		static void Cull(int32_t Bit)
		{
			s_RendererAPI->Cull(Bit);
		}

		static void CullFrontOrBack(bool bFront)
		{
			s_RendererAPI->CullFrontOrBack(bFront);
		}

		// framebuffer
        static int GetDrawFrameBuffer()
		{
			return s_RendererAPI->GetDrawFrameBuffer();
		}

		static void BindFrameBuffer(uint32_t framebufferID)
		{
			s_RendererAPI->BindFrameBuffer(framebufferID);
		}

    private:
        static Scope<RendererAPI> s_RendererAPI;
    };


}
