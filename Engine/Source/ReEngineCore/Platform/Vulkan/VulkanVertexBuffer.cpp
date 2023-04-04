#include "VulkanVertexBuffer.h"

//InputBinding标记如何去识别当前bind上该以怎么样的速度去更新数据块bind的数据块
//比如我如果把两个属性存在两个Buffer里，那么我就需要生成两个BindingDecription
//其中每一个Binding的stride都会是对应的属性的大小
//然后会根据速率，在管线中对bind所指代的数据切换
VkVertexInputBindingDescription VulkanVertexBuffer::GetInputBinding()
{
    int32 stride = 0;
    for (int32 i = 0; i < Attributes.size(); ++i)
    {
        stride += VertexAttributeToSize(Attributes[i]);
    }

    VkVertexInputBindingDescription vertexInputBinding = {};
    vertexInputBinding.binding   = 0;
    vertexInputBinding.stride    = stride;
    vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return vertexInputBinding;
}

//VertexInputAttr标记如何把采样到的数据和VertexShader中的定义对上
//标记如何把Bind中的数据和VertexShader中的定义对上
//如果保存在两个buffer中，那么我们就需要分成两个bind，location可以不用变，format不用变，offset需要变
//bind可以理解为几个区块，input具体的区块中的具体的位置
std::vector<VkVertexInputAttributeDescription> VulkanVertexBuffer::GetInputAttributes(const std::vector<VertexAttribute>& shaderInputs)
{
    std::vector<VkVertexInputAttributeDescription> vertexInputAttributs;
    int32 offset = 0;

    if(shaderInputs.size() > 0)
    {
        for (int32 i = 0; i < shaderInputs.size(); ++i)
        {
            VkVertexInputAttributeDescription inputAttribute = {};
            inputAttribute.binding  = 0; //是指管线绑定的VertexBuffer中的哪一个
            inputAttribute.location = i; //Location是属性的位置
            inputAttribute.format   = VertexAttributeToVkFormat(shaderInputs[i]);
            inputAttribute.offset   = offset;
            offset += VertexAttributeToSize(shaderInputs[i]);
            vertexInputAttributs.push_back(inputAttribute);
        }
    }
    else
    {
        for (int32 i = 0; i < Attributes.size(); ++i)
        {
            VkVertexInputAttributeDescription inputAttribute = {};
            inputAttribute.binding  = 0; //是指管线绑定的VertexBuffer中的哪一个
            inputAttribute.location = i; //Location是属性的位置
            inputAttribute.format   = VertexAttributeToVkFormat(Attributes[i]);
            inputAttribute.offset   = offset;
            offset += VertexAttributeToSize(Attributes[i]);
            vertexInputAttributs.push_back(inputAttribute);
        }
    }
    
    return vertexInputAttributs;
}

VulkanVertexBuffer* VulkanVertexBuffer::Create(std::shared_ptr<VulkanDevice> device, VulkanCommandBuffer* cmdBuffer,std::vector<float> vertices, const std::vector<VertexAttribute>& attributes)
{
    VulkanVertexBuffer* VertexBuffer = new VulkanVertexBuffer();

    VertexBuffer->Device = device->GetInstanceHandle();
    VertexBuffer->Attributes = attributes;
    
    VkDeviceSize VertexbufferSize = vertices.size() * sizeof(float);

    VulkanBuffer* stagingIndexBuffer = VulkanBuffer::CreateBuffer(
            device,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            VertexbufferSize,(void*)vertices.data());
    
    VertexBuffer->Buffer = VulkanBuffer::CreateBuffer(
             device,
             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ,
             VertexbufferSize);

    VulkanBuffer::TransferBuffer(device,cmdBuffer,stagingIndexBuffer,VertexBuffer->Buffer,VertexbufferSize);
    
    delete stagingIndexBuffer;
    
    return VertexBuffer;
}
