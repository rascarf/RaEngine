﻿#include "Core/PCH.h"
#include "Renderer2D.h"

#include <array>

#include "glm/gtx/transform.hpp"
#include "Library/ShaderLibrary.h"
#include "RHI/Renderer.h"
#include "RHI/Shader.h"
#include "RHI/VertexArray.h"


namespace ReEngine
{
    struct QuadVertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texCoord;
        float texIndex;
        float tilingFactor;
    };
    
    struct Renderer2DData
    {
        static constexpr  uint32_t MaxQuads = 20000;
        static constexpr uint32_t MaxVertices = MaxQuads * 4;
        static constexpr uint32_t MaxIndices = MaxQuads * 6;
        static constexpr uint32_t MaxTextureSlots = 32;

        Ref<VertexArray> QuadVertexArray;
        Ref<VertexBuffer> QuadVertexBuffer;
        Ref<Shader> TextureShader;
        Ref<Texture2D> WhiteTexture;

        uint32_t QuadIndexCount = 0;
        QuadVertex* QuadVertexBufferBase = nullptr;
        QuadVertex* QuadVertexBufferPtr = nullptr;

        std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = white texture

        glm::vec4 QuadVertexPositions[4];
        
    };

    static Scope<Renderer2DData> s_Data;


    void Renderer2D::Init()
    {
        s_Data = CreateScope<Renderer2DData>();
        s_Data->QuadVertexArray = VertexArray::Create();
        s_Data->QuadVertexBuffer = VertexBuffer::Create(s_Data->MaxVertices * sizeof(QuadVertex));
        s_Data->QuadVertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float4, "a_Color" },
            { ShaderDataType::Float2, "a_TexCoord" },
            { ShaderDataType::Float, "a_TexIndex" },
            { ShaderDataType::Float, "a_TilingFactor" }
            });

        s_Data->QuadVertexArray->AddVertexBuffer(s_Data->QuadVertexBuffer);
        s_Data->QuadVertexBufferBase = new QuadVertex[s_Data->MaxVertices];
        
        uint32_t* quadIndices = new uint32_t[s_Data->MaxIndices];

        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_Data->MaxIndices; i += 6)
        {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }

        Ref<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, s_Data->MaxIndices);
        s_Data->QuadVertexArray->SetIndexBuffer(quadIB);
        delete[] quadIndices;
        
        s_Data->WhiteTexture = Texture2D::Create(1,1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

        int32_t samplers[32];
        for (uint32_t i = 0; i < s_Data->MaxTextureSlots; i++)
            samplers[i] = (int32_t)i;
        
        s_Data->TextureShader = Library<Shader>::GetInstance().Get("TextureShader");;
        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetIntArray("u_Textures", samplers, s_Data->MaxTextureSlots);

        s_Data->TextureSlots[0] = s_Data->WhiteTexture;

        s_Data->QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
        s_Data->QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
        s_Data->QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
        s_Data->QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };
    }

    void Renderer2D::ShutDown()
    {
        s_Data.release();
    }

    void Renderer2D::BeginScene(const OrthographicCamera& camera)
    {
        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());

        s_Data->QuadIndexCount = 0;
        s_Data->QuadVertexBufferPtr = s_Data->QuadVertexBufferBase;

        s_Data->TextureSlotIndex = 1;
    }

    void Renderer2D::BeginScene(const Camera& camera,const glm::mat4& transform)
    {
        glm::mat4 viewProj = camera.GetProjection() * glm::inverse(transform);
        
        s_Data->TextureShader->Bind();
        s_Data->TextureShader->SetMat4("u_ViewProjection", viewProj);

        s_Data->QuadIndexCount = 0;
        s_Data->QuadVertexBufferPtr = s_Data->QuadVertexBufferBase;

        s_Data->TextureSlotIndex = 1;
    }

    void Renderer2D::EndScene()
    {
        uint32_t dataSize = (uint32_t)( (uint8_t*)s_Data->QuadVertexBufferPtr - (uint8_t*)s_Data->QuadVertexBufferBase );
        s_Data->QuadVertexBuffer->SetData(s_Data->QuadVertexBufferBase, dataSize);

        Flush();
    }

    void Renderer2D::Flush()
    {
        if(s_Data->QuadIndexCount ==0)
            return;

        for (uint32_t i = 0; i < s_Data->TextureSlotIndex; i++)
            s_Data->TextureSlots[i]->Bind(i);
		
        RenderCommand::DrawIndexed(s_Data->QuadVertexArray, s_Data->QuadIndexCount);
    }

    
    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
    {
        DrawQuad({ position.x, position.y, 0.0f }, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture,float tilingFactor, const glm::vec4& tintColor)
    {
        DrawQuad({position.x,position.y,0.0f},size,texture,tilingFactor,tintColor);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
    {
        constexpr size_t quadVertexCount = 4;
        const float textureIndex = 0.0f; // White Texture
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
        const float tilingFactor = 1.0f;

        if (s_Data->QuadIndexCount >= Renderer2DData::MaxIndices)
            FlushAndReset();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data->QuadVertexBufferPtr->position = transform * s_Data->QuadVertexPositions[i];
            s_Data->QuadVertexBufferPtr->color = color;
            s_Data->QuadVertexBufferPtr->texCoord = textureCoords[i];
            s_Data->QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data->QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data->QuadVertexBufferPtr++;
        }

        s_Data->QuadIndexCount += 6;
    }
    
    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture,float tilingFactor, const glm::vec4& tintColor)
    {
        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        if (s_Data->QuadIndexCount >= Renderer2DData::MaxIndices)
            FlushAndReset();

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++)
        {
            if (*s_Data->TextureSlots[i].get() == *texture.get())
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0.0f)
        {
            if (s_Data->TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
                FlushAndReset();

            textureIndex = (float)s_Data->TextureSlotIndex;
            s_Data->TextureSlots[s_Data->TextureSlotIndex] = texture;
            s_Data->TextureSlotIndex++;
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data->QuadVertexBufferPtr->position = transform * s_Data->QuadVertexPositions[i];
            s_Data->QuadVertexBufferPtr->color = tintColor;
            s_Data->QuadVertexBufferPtr->texCoord = textureCoords[i];
            s_Data->QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data->QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data->QuadVertexBufferPtr++;
        }

        s_Data->QuadIndexCount += 6;
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<SubTexture2D>& texture,
        float tilingFactor, const glm::vec4& tintColor)
    {
        DrawQuad({position.x,position.y,0.0f},size,texture,tilingFactor,tintColor);
    }

    void Renderer2D::DrawRotateQuad(const glm::vec2& position, const glm::vec2& size, float Rotation,
     const Ref<SubTexture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        DrawRotateQuad({position.x,position.y,0.0f},size,Rotation,texture,tilingFactor,tintColor);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<SubTexture2D>& subtexture,float tilingFactor, const glm::vec4& tintColor)
    {
        constexpr size_t quadVertexCount = 4;
        // constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
        const  glm::vec2* textureCoords = subtexture->GetTexCoords();
        auto texture = subtexture->GetTexture();
        
        if (s_Data->QuadIndexCount >= Renderer2DData::MaxIndices)
            FlushAndReset();

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++)
        {
            if (*s_Data->TextureSlots[i].get() == *texture.get())
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0.0f)
        {
            if (s_Data->TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
                FlushAndReset();

            textureIndex = (float)s_Data->TextureSlotIndex;
            s_Data->TextureSlots[s_Data->TextureSlotIndex] = texture;
            s_Data->TextureSlotIndex++;
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data->QuadVertexBufferPtr->position = transform * s_Data->QuadVertexPositions[i];
            s_Data->QuadVertexBufferPtr->color = tintColor;
            s_Data->QuadVertexBufferPtr->texCoord = textureCoords[i];
            s_Data->QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data->QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data->QuadVertexBufferPtr++;
        }

        s_Data->QuadIndexCount += 6;
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color)
    {
        constexpr size_t quadVertexCount = 4;
        const float textureIndex = 0.0f; // White Texture
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
        const float tilingFactor = 1.0f;

        if (s_Data->QuadIndexCount >= Renderer2DData::MaxIndices)
            FlushAndReset();

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data->QuadVertexBufferPtr->position = transform * s_Data->QuadVertexPositions[i];
            s_Data->QuadVertexBufferPtr->color = color;
            s_Data->QuadVertexBufferPtr->texCoord = textureCoords[i];
            s_Data->QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data->QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data->QuadVertexBufferPtr++;
        }

        s_Data->QuadIndexCount += 6;
    }

    void Renderer2D::DrawRotateQuad(const glm::vec3& position, const glm::vec2& size, float Rotation,const Ref<SubTexture2D>& subtexture, float tilingFactor, const glm::vec4& tintColor)
    {
        constexpr size_t quadVertexCount = 4;
        // constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
        const  glm::vec2* textureCoords = subtexture->GetTexCoords();
        auto texture = subtexture->GetTexture();
        
        if (s_Data->QuadIndexCount >= Renderer2DData::MaxIndices)
            FlushAndReset();

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++)
        {
            if (*s_Data->TextureSlots[i].get() == *texture.get())
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0.0f)
        {
            if (s_Data->TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
                FlushAndReset();

            textureIndex = (float)s_Data->TextureSlotIndex;
            s_Data->TextureSlots[s_Data->TextureSlotIndex] = texture;
            s_Data->TextureSlotIndex++;
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(Rotation), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data->QuadVertexBufferPtr->position = transform * s_Data->QuadVertexPositions[i];
            s_Data->QuadVertexBufferPtr->color = tintColor;
            s_Data->QuadVertexBufferPtr->texCoord = textureCoords[i];
            s_Data->QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data->QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data->QuadVertexBufferPtr++;
        }

        s_Data->QuadIndexCount += 6;
    }

    void Renderer2D::DrawRotateQuad(const glm::vec2& position, const glm::vec2& size, float Rotation,const glm::vec4& color)
    {
        DrawRotateQuad({ position.x, position.y, 0.0f }, size, Rotation, color);
    }
    
    void Renderer2D::DrawRotateQuad(const glm::vec2& position, const glm::vec2& size, float Rotation,const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        DrawRotateQuad({ position.x, position.y, 0.0f }, size, Rotation, texture, tilingFactor, tintColor);
    }
    
    void Renderer2D::DrawRotateQuad(const glm::vec3& position, const glm::vec2& size, float Rotation,const glm::vec4& color)
    {
        constexpr size_t quadVertexCount = 4;
        const float textureIndex = 0.0f; // White Texture
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
        const float tilingFactor = 1.0f;

        if (s_Data->QuadIndexCount >= Renderer2DData::MaxIndices)
            FlushAndReset();

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(Rotation), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data->QuadVertexBufferPtr->position = transform * s_Data->QuadVertexPositions[i];
            s_Data->QuadVertexBufferPtr->color = color;
            s_Data->QuadVertexBufferPtr->texCoord = textureCoords[i];
            s_Data->QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data->QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data->QuadVertexBufferPtr++;
        }

        s_Data->QuadIndexCount += 6;
    }
    
    void Renderer2D::DrawRotateQuad(const glm::vec3& position, const glm::vec2& size, float Rotation,const Ref<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
    {
        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

        if (s_Data->QuadIndexCount >= Renderer2DData::MaxIndices)
            FlushAndReset();

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++)
        {
            if (*s_Data->TextureSlots[i].get() == *texture.get())
            {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0.0f)
        {
            if (s_Data->TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
                FlushAndReset();

            textureIndex = (float)s_Data->TextureSlotIndex;
            s_Data->TextureSlots[s_Data->TextureSlotIndex] = texture;
            s_Data->TextureSlotIndex++;
        }

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::rotate(glm::mat4(1.0f), glm::radians(Rotation), { 0.0f, 0.0f, 1.0f })
            * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

        for (size_t i = 0; i < quadVertexCount; i++)
        {
            s_Data->QuadVertexBufferPtr->position = transform * s_Data->QuadVertexPositions[i];
            s_Data->QuadVertexBufferPtr->color = tintColor;
            s_Data->QuadVertexBufferPtr->texCoord = textureCoords[i];
            s_Data->QuadVertexBufferPtr->texIndex = textureIndex;
            s_Data->QuadVertexBufferPtr->tilingFactor = tilingFactor;
            s_Data->QuadVertexBufferPtr++;
        }

        s_Data->QuadIndexCount += 6;
    }

    void Renderer2D::FlushAndReset()
    {
        EndScene();

        s_Data->QuadIndexCount = 0;
        s_Data->QuadVertexBufferPtr = s_Data->QuadVertexBufferBase;
        s_Data->TextureSlotIndex = 1;
    }
}
