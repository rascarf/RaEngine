#pragma once
#include "SceneGraph.h"
#include "Core/Core.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanBuffer.h"

namespace ReEngine
{
    // struct FrameGraph;

    static const uint16 k_invalid_scene_texture_index      = UINT16_MAX;
    static const uint32 k_material_descriptor_set_index    = 1;

    static bool recreate_per_thread_descriptors = false;
    static bool use_secondary_command_buffers   = false;
    
    enum DrawFlags
    {
        DrawFlags_AlphaMask     = 1 << 0,
        DrawFlags_DoubleSided   = 1 << 1,
        DrawFlags_Transparent   = 1 << 2,
    };

    struct GpuSceneData
    {
        glm::mat4 ViewPorjection;
        glm::vec4 Eye;
        glm::vec4 LightPostion;
        float    LightRange;
        float    LightIntensity;
        float    Padding[2];
    };

    // Render的结构
    class RenderScene
    {
        virtual ~RenderScene(){}

        virtual void Init(cstring FileName,cstring Path){};
        virtual void ShutDown(){};

        // virtual void RegisterRenderPasses(FrameGraph* FrameGraph){};
        virtual void PrepareDraws(){};

        virtual void UploadMaterials(){}
        virtual void SubmitDrawTask(){}

        Ref<SceneGraph>     SceneGraph;
        Ref<VulkanBuffer>   SceneCB;

        float               GlobalScale = 1.0f;
    };
    
}
