#pragma once
#include "Camera/OrthographicCamera.h"
#include "Renderer/RHI/Texture.h"

namespace ReEngine
{
    class Renderer2D
    {
    public:
        static void Init();
        static void ShutDown();

        static void BeginScene(const OrthographicCamera& camera);
        static void EndScene();
        static void Flush();


        //Primitives
        static void DrawQuad(const glm::vec2& position,const glm::vec2& size, const glm::vec4& color);
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
        
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture,float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture,float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

        static void DrawRotateQuad(const glm::vec2& position, const glm::vec2& size,float Rotation,const glm::vec4& color);
        static void DrawRotateQuad(const glm::vec3& position, const glm::vec2& size,float Rotation,const glm::vec4& color);
        static void DrawRotateQuad(const glm::vec2& position, const glm::vec2& size,float Rotation,const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
        static void DrawRotateQuad(const glm::vec3& position, const glm::vec2& size,float Rotation,const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
    private:
        static void FlushAndReset();
    };
}
