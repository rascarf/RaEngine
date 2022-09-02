#include "SandBoxLayer.h"
#include "ReEngine.h"

SandBoxLayer::SandBoxLayer():Layer("SandBoxLayer"),m_CameraController(1280.0f / 720.0f)
{
    mVertexArray = ReEngine::VertexArray::Create();
    float vertices[3 * 7] = {
        -0.5f, -0.5f, 0.0f, 0.8f, 0.2f, 0.8f, 1.0f,
         0.5f, -0.5f, 0.0f, 0.2f, 0.3f, 0.8f, 1.0f,
         0.0f,  0.5f, 0.0f, 0.8f, 0.8f, 0.2f, 1.0f
    };
    auto VertexBuffer = ReEngine::VertexBuffer::Create(vertices, sizeof(vertices));
    ReEngine::BufferLayout layout = {
        { ReEngine::ShaderDataType::Float3, "a_Position" },
        { ReEngine::ShaderDataType::Float4, "a_Color" }
    };
    VertexBuffer->SetLayout(layout);
    mVertexArray->AddVertexBuffer(VertexBuffer);
    uint32_t indices[3] = { 0, 1, 2 };
    std::shared_ptr<ReEngine::IndexBuffer> indexBuffer;
    indexBuffer= ReEngine::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));
    mVertexArray->SetIndexBuffer(indexBuffer);

    mSquareVA= ReEngine::VertexArray::Create();
    float squareVertices[3 * 4] = {
        -0.75f, -0.75f, 0.0f,
         0.75f, -0.75f, 0.0f,
         0.75f,  0.75f, 0.0f,
        -0.75f,  0.75f, 0.0f
    };
    std::shared_ptr<ReEngine::VertexBuffer> squareVB;
    squareVB= ReEngine::VertexBuffer::Create(squareVertices, sizeof(squareVertices));
    squareVB->SetLayout({
        { ReEngine::ShaderDataType::Float3, "a_Position" }
    });
    mSquareVA->AddVertexBuffer(squareVB);
    uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
    std::shared_ptr<ReEngine::IndexBuffer> squareIB;
    squareIB = ReEngine::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
    mSquareVA->SetIndexBuffer(squareIB);

    std::string vertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;
			layout(location = 1) in vec4 a_Color;

			uniform mat4 u_ViewProjection;

			out vec3 v_Position;
			out vec4 v_Color;

			void main()
			{
				v_Position = a_Position;
				v_Color = a_Color;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);	
			}
		)";

    std::string fragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;
			in vec4 v_Color;

			void main()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0);
				color = v_Color;
			}
		)";

	mShader = ReEngine::Shader::Create("Test",vertexSrc, fragmentSrc);

	std::string blueShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * vec4(a_Position, 1.0);	
			}
		)";

	std::string blueShaderFragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			void main()
			{
				color = vec4(0.2, 0.3, 0.8, 1.0);
			}
		)";

	mBlueShader = ReEngine::Shader::Create("TestSqure",blueShaderVertexSrc, blueShaderFragmentSrc);
	
	mTetxureVA = ReEngine::VertexArray::Create();
	float TexturesquareVertices[5 * 4] = {
		-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
		 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
		-0.5f,  0.5f, 0.0f, 0.0f, 1.0f
	};
	ReEngine::Ref<ReEngine::VertexBuffer> TexturesquareVB;
	TexturesquareVB = ReEngine::VertexBuffer::Create(TexturesquareVertices, sizeof(TexturesquareVertices));
	TexturesquareVB->SetLayout({
			{ ReEngine::ShaderDataType::Float3, "a_Position" },
			{ ReEngine::ShaderDataType::Float2, "a_TexCoord" }
		});
	mTetxureVA->AddVertexBuffer(TexturesquareVB);
	mTetxureVA->SetIndexBuffer(squareIB);
	
	mTetxureShader = ReEngine::Library<ReEngine::Shader>::GetInstance().Get("TextureShader");
	mTexture = ReEngine::Texture2D::Create(std::string("assets/Textures/Checkerboard.png"));

	mTetxureShader->Bind();
	mTetxureShader->SetInt("u_Texture", 0);
}

SandBoxLayer::~SandBoxLayer()
{
    
}

void SandBoxLayer::OnAttach()
{
    
	
    
}

void SandBoxLayer::OnDetach()
{
    
}

void SandBoxLayer::OnEvent(std::shared_ptr<ReEngine::Event> e)
{
    m_CameraController.OnEvent(e);
}

void SandBoxLayer::OnUpdate(ReEngine::Timestep ts)
{
	m_CameraController.OnUpdate(ts);

	ReEngine::RenderCommand::Clear();
	ReEngine::Renderer::BeginScene(m_CameraController.GetCamera());
	ReEngine::Renderer::Submit(mBlueShader, mSquareVA);

	mTexture->Bind();
	ReEngine::Renderer::Submit(mTetxureShader, mTetxureVA);
	ReEngine::Renderer::Submit(mShader, mVertexArray);
	
	ReEngine::Renderer::EndScene();
}
