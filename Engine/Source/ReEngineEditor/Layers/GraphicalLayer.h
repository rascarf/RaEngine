#pragma once

#include "Layer/Layer.h"
#include "Platform/Vulkan/VulkanPipelineInfo.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanDynamicBufferRing.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanFrameBuffer.h"

class GraphicalLayer : public ReEngine::Layer
{
public:
    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual void OnEvent(std::shared_ptr<ReEngine::Event> e) override;
    virtual void OnUpdate(ReEngine::Timestep ts) override;

    virtual void OnRender() override;
    virtual void OnUIRender(ReEngine::Timestep ts) override;

    virtual void OnCreateBackBuffer();
    virtual void OnInit(){}
    virtual void OnDeInit(){}
    virtual void OnChangeWindowSize(std::shared_ptr<ReEngine::Event> e);

protected:
    ReEngine::VulkanContext* VkContext;
    Ref<VulkanBackBuffer> FrameBuffer;
};

struct ModelBlock
{
    glm::mat4 model;
};

struct ViewProjectionBlock
{
    glm::mat4 view;
    glm::mat4 projection;
};

struct MVPBlock
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

struct AttachmentParamBlock
{
    int attachmentIndex = 0;
    float zNear = 0.1f;
    float zFar = 3000.0f;
    float padding;
};

struct PointLight
{
    glm::vec3 Position;
    float Radius;
    glm::vec3 Color;
    float Padding;

    PointLight()
    {

    }

    PointLight(const glm::vec3& inPosition, float inRadius, const glm::vec3& inColor)
        : Position(inPosition)
        , Radius(inRadius)
        , Color(inColor)
    {

    }
};