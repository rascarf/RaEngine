#include "SandBoxLayer.h"
#include "imgui.h"
#include "ReEngine.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "Platform/Vulkan/VulkanContext.h"
#include <Shader_frag.h>
#include <Shader_vert.h>


SandBoxLayer::SandBoxLayer()
{
	
}

SandBoxLayer::~SandBoxLayer()
{
}

void SandBoxLayer::OnUpdate(ReEngine::Timestep ts)
{
	RingBuffer->OnBeginFrame();
	UpdateUniformBuffer(ts);
}

void SandBoxLayer::OnRender()
{
	auto Context = Renderer::GetContext().get();
	auto VulkanContext = dynamic_cast<ReEngine::VulkanContext*>(Context);
	
	VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    vkBeginCommandBuffer(VulkanContext->GetCommandList(), &beginInfo);

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = FrameBuffer->m_RenderPass;
    renderPassInfo.framebuffer = FrameBuffer->m_FrameBuffers[VulkanContext->GetCurrtIndex()];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = VkExtent2D(VulkanContext->Instance->GetSwapChain()->GetWidth(),VulkanContext->Instance->GetSwapChain()->GetHeight());

    VkClearValue clearValues[2];
    clearValues[0].color        = {{0.2f, 0.2f, 0.2f, 1.0f}};
    clearValues[1].depthStencil = { 1.0f, 0 };
    
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;
    
    vkCmdBeginRenderPass(VkContext->GetCommandList(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = {};
    viewport.x        = 0;
    viewport.y        = VkContext->WinProperty->Height;
    viewport.width    = VkContext->WinProperty->Width;
    viewport.height   = -(float)VkContext->WinProperty->Height;    // flip y axis
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.extent.width  = VkContext->WinProperty->Width;
    scissor.extent.height = VkContext->WinProperty->Height;
    scissor.offset.x      = 0;
    scissor.offset.y      = 0;

    vkCmdSetViewport(VkContext->GetCommandList(), 0, 1, &viewport);
    vkCmdSetScissor(VkContext->GetCommandList(), 0, 1, &scissor);
    vkCmdBindPipeline(VkContext->GetCommandList(), VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline->Pipeline);

    const auto ParamBufferView =  RingBuffer->AllocConstantBuffer(sizeof(ParamBlock),&Param);
    
    for (int32 meshIndex = 0; meshIndex < Model->Meshes.size(); ++meshIndex)
    {
        auto BufferView = RingBuffer->AllocConstantBuffer(sizeof(UniformBufferObject),&ubo);
    	
    	PipeSet->WriteBindOffset("uboMVP",BufferView.offset);
    	PipeSet->WriteBindOffset("params",ParamBufferView.offset);
    	
        PipeSet->BindSet(VkContext->GetCommandList(),PipeShader->pipelineLayout);
    	
        Model->Meshes[meshIndex]->BindDraw(VkContext->GetCommandList());
    }
    
    vkCmdEndRenderPass(VkContext->GetCommandList());
}


void SandBoxLayer::OnUIRender(ReEngine::Timestep ts)
{
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::Begin("Texture", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		ImGui::Text("SSS Demo");

		for (int32 i = 0; i < Model->Meshes.size(); ++i)
		{
			Ref<VulkanMesh> mesh = Model->Meshes[i];
			ImGui::Text("%-20s Tri:%d", mesh->LinkNode.lock()->name.c_str(), mesh->TriangleCount);
		}

		ImGui::SliderFloat("Curvature", &(Param.curvature),       0.0f, 10.0f);
		ImGui::SliderFloat2("CurvatureBias", (float*)&(Param.curvatureScaleBias), 0.0f, 1.0f);

		ImGui::SliderFloat("BlurredLevel", &(Param.blurredLevel), 0.0f, 12.0f);
		ImGui::SliderFloat("Exposure", &(Param.exposure),         0.0f, 10.0f);

		ImGui::SliderFloat3("LightDirection", (float*)&(Param.lightDir), -10.0f, 10.0f);
		ImGui::ColorEdit3("LightColor", (float*)&(Param.lightColor));

		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		BoundingBox Box = Model->RootNode->GetBounds();
		glm::vec3 boundSize   = Box.Max - Box.Min;
		glm::vec3 boundCenter = Box.Min + boundSize * 0.5f;
		ImGui::InputFloat3("Mesh", &(boundCenter.r));
    		
		auto CameraPostion = Camera->GetPosition();
		ImGui::InputFloat3("CameraPosition", &(CameraPostion.r));
    		
		ImGui::End();
	}
}

void SandBoxLayer::OnChangeWindowSize(std::shared_ptr<ReEngine::Event> e)
{
	CreateGraphicsPipeline();
}

void SandBoxLayer::OnInit()
{
	Camera = CreateRef<EditorCamera>();
	CreateMeshBuffer();
	CreateGraphicsPipeline();
}

void SandBoxLayer::OnDeInit()
{
	Model.reset();
    	
	PipeShader.reset();
	RingBuffer.reset();
    	
	TexCurve.reset();
	TexDiffuse.reset();
	TexNomal.reset();
	TexPreIntegareted.reset();
	GraphicsPipeline.reset();
    	
	FrameBuffer->ShutDown();
}

void SandBoxLayer::CreateGraphicsPipeline()
{
	VulkanPipelineInfo DefaultInfo;
	DefaultInfo.Shader = PipeShader;
    	
	GraphicsPipeline = VulkanPipeline::Create(
		VkContext->Instance->GetDevice(),
		VkContext->CommandPool->m_PipelineCache,
		DefaultInfo,
		{Model->GetInputBinding()},
		Model->GetInputAttributes(),
		PipeShader->pipelineLayout,
		FrameBuffer->m_RenderPass
	);
}

void SandBoxLayer::CreateMeshBuffer()
{
	auto cmd = VulkanCommandBuffer::Create(VkContext->Instance->GetDevice(),VkContext->CommandPool->m_CommandPool);
    	
	Model = VulkanModel::LoadFromFile("Assets/Mesh/head.obj",
		VkContext->Instance->GetDevice(),
		cmd,
		{ VertexAttribute::VA_Position, VertexAttribute::VA_UV0, VertexAttribute::VA_Normal, VertexAttribute::VA_Tangent});

	TexDiffuse = VulkanTexture::Create2D(
	"Assets/Textures/head_diffuse.jpg",VkContext->Instance->GetDevice(),cmd
	);
    	
	TexPreIntegareted = VulkanTexture::Create2D(
"Assets/Textures/preIntegratedLUT.png",VkContext->Instance->GetDevice(),cmd
	);
    	
	TexCurve = VulkanTexture::Create2D(
"Assets/Textures/curvatureLUT.png",VkContext->Instance->GetDevice(),cmd
	);

	TexNomal = VulkanTexture::Create2D(
	"Assets/Textures/head_normal.jpg",VkContext->Instance->GetDevice(),cmd
	);
    	
	PipeShader = VulkanShader::Create(VkContext->Instance->GetDevice(),true,&SHADER_VERT,&SHADER_FRAG,nullptr,nullptr,nullptr,nullptr);
	PipeSet = PipeShader->AllocateDescriptorSet();

	RingBuffer = CreateRef<VulkanDynamicBufferRing>();
	RingBuffer->OnCreate(VkContext->Instance->GetDevice(),3,200 * 1024 * 1024);

	PipeSet->WriteBuffer("uboMVP",RingBuffer->GetSetDescriptor(sizeof(UniformBufferObject)));
	PipeSet->WriteBuffer("params",RingBuffer->GetSetDescriptor(sizeof(ParamBlock)));
    	
	PipeSet->WriteImage("diffuseMap",TexDiffuse);
	PipeSet->WriteImage("normalMap",TexNomal);
	PipeSet->WriteImage("curvatureMap",TexPreIntegareted);
	PipeSet->WriteImage("preIntegratedMap",TexCurve);

	ubo.model = glm::mat4(1.0f);
	ubo.view = glm::mat4(1.0f);
	ubo.proj = glm::mat4(1.0f);

	Param.blurredLevel = 2.0;
	Param.curvature = 3.5;
	Param.curvatureScaleBias.x = 0.101;
	Param.curvatureScaleBias.y = -0.001;
	Param.exposure = 1.0;
	Param.lightColor = glm::vec3(240.0f / 255.0f, 200.0f / 255.0f, 166.0f / 255.0f);
	Param.lightDir = glm::vec3(1, 0, -1.0);
	Param.lightDir = normalize(Param.lightDir);
	Param.padding = 0.0;
}

void SandBoxLayer::UpdateUniformBuffer(Timestep ts)
{
	Camera->OnUpdate(ts);
	ubo.model = glm::rotate(ubo.model, ts.GetSeconds() *  glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));;
	ubo.view = Camera->GetViewMatrix();
	ubo.proj = Camera->GetProjection();
}

