#include "Renderer.h"

#include "RenderCommand.h"
#include "Core/Application.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace ReEngine
{
   Renderer::SceneData* Renderer::mSceneData  = new Renderer::SceneData;

   void Renderer::Init()
   {
      RenderCommand::Init();
   }

   void Renderer::Shutdown()
   {
      
   }

   void Renderer::OnWindowResize(uint32_t width, uint32_t height)
   {
      RenderCommand::SetViewport(0,0,width,height);
   }

   void Renderer::BeginScene(OrthographicCamera camera)
   {
      mSceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
   }

   void Renderer::EndScene()
   {
      
   }
   

   void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
   {
      shader->Bind();
      std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewProjection", mSceneData->ViewProjectionMatrix);
      std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);

      vertexArray->Bind();
      RenderCommand::DrawIndexed(vertexArray);
   }

   Ref<GraphicsContext> Renderer::GetContext()
   {
      return Application::GetInstance().GetWindow().GetGraphicsContext();
   }


   
}
