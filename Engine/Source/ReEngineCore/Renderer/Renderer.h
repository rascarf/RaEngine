#pragma once

#include "Shader.h"
#include "Renderer/GraphicsContext.h"
#include "RenderCommand.h"


namespace ReEngine
{
    class Renderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void OnWindowResize(uint32_t width, uint32_t height);
        static void EndScene();
        static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

        static Ref<GraphicsContext> GetContext();

    private:
        struct SceneData
        {
            glm::mat4 ViewProjectionMatrix;
        };

        static SceneData* mSceneData;
    };
}
