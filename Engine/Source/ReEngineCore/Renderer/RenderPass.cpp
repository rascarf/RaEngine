#include "RenderPass.h"

namespace ReEngine
{
    // uint32_t RenderPass::ExcuteAndReturnFinalTex()
    // {
    //     PostProcessing::mIntermediateScreenTex = Texture2D::Create(
    //         mSpecification.TargetFramebuffer->GetSpecification().Width, 
    //         mSpecification.TargetFramebuffer->GetSpecification().Height
    //     );
    //
    //     uint32_t final = 0;
    //     for (auto& effect : mPostProcessings)
    //     {
    //         final = effect->ExcuteAndReturnFinalTex(mSpecification.TargetFramebuffer);
    //     }
    //     return final;
    // }
}