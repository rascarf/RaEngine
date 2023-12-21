#pragma once
#include "Core/Core.h"
#include "Platform/Vulkan/VulkanRenderTarget.h"
#include "Platform/Vulkan/Mesh/VulkanMesh.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanBuffers/VulkanTexture.h"
#include "RenderGraph/RenderScene.h"

namespace ReEngine
{
    enum class RenderPassOperation
    {  
        DontCare = 0, Load = 1, Clear = 2, Count = 3
    }; // enum Enum
    
    typedef uint32 FrameGraphHandle;
    static const uint32                    k_invalid_index = 0xffffffff;
    
    struct FrameGraphResourceHandle
    {
        FrameGraphHandle Index;
    };

    struct FrameGraphNodeHandle
    {
        FrameGraphHandle Index;
    };

    // Resource的种类
    enum FrameGraphResourceType
    {
        FrameGraphResourceType_Invalid = -1,
        FrameGraphResourceType_Buffer = 0,
        FrameGraphResourceType_Texture = 1,
        FrameGraphResourceType_Attachment = 2,
        FrameGraphResourceType_Reference = 3,
    };

    struct FrameGraphBuffer
    {
        size_t Size = 0;
                
        VkBufferUsageFlags Flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        Ref<VulkanBuffer> Buffer = nullptr;
                
    };

    struct FrameGraphTexture
    {
        uint32 Width = 0;
        uint32 Height = 0;
        uint32 Depth = 0;

        VkFormat Format = VK_FORMAT_R8_SINT;
        VkImageUsageFlags UsageFlags = VK_IMAGE_TYPE_2D;
        RenderPassOperation LoadOp = RenderPassOperation::Count;

        Ref<VulkanTexture> Texture = nullptr;
    };

    // Resource的描述信息
    class FrameGraphResourceInfo
    {
    public:
        bool External = false;
        FrameGraphBuffer Buffer;
        FrameGraphTexture Texture;
    };

    // Input可以是Texture或者Attachment
    // 如果是一个Attachment，我们需要控制是否需要丢弃之前的内容
    // 比如现在是我们第一次使用，我们需不需要Load这个数据
    // Output总是实现store op
    struct FrameGraphResource
    {
        FrameGraphResourceType ResourceType = FrameGraphResourceType::FrameGraphResourceType_Invalid;
        FrameGraphResourceInfo ResourceInfo {};

        FrameGraphNodeHandle Producer {};
        FrameGraphResourceHandle OutputHandle {};

        int32 RefCount = 0;
        string Name;
    };

    struct FrameGraphResourceInputCreation
    {
        FrameGraphResourceType ResourceType;
        FrameGraphResourceInfo ResourceInfo;
        string Name;
    };

    class FrameGraphResourceOutputCreation
    {
    public:
        
        FrameGraphResourceType ResourceType;
        FrameGraphResourceInfo ResourceInfo;
        string Name;
    };

    struct FrameGraphNodeCreation
    {
        std::vector<FrameGraphResourceInputCreation> Inputs;
        std::vector<FrameGraphResourceOutputCreation> Outputs;

        bool Enabled;
        string Name;
    };

    struct FrameGraphRenderPass
    {
        virtual void PreRender(Ref<VulkanCommandBuffer> Cmd,Ref<VulkanModel> Model){}
        virtual void Render(Ref<VulkanCommandBuffer> Cmd,Ref<VulkanModel> Model){}
        virtual void OnResize(){}

        virtual ~FrameGraphRenderPass() = default;
    };

    struct FrameGraphNode
    {
        int32 RefCount = 0;

        Ref<VulkanRenderTarget> RenderTarget;

        Ref<FrameGraphRenderPass> GraphRenderPass;
        
        std::vector<FrameGraphResourceHandle> Inputs;
        std::vector<FrameGraphResourceHandle> Outputs;

        std::vector<FrameGraphNodeHandle> Edges;

        bool Enabled = true;
        string Name;
    };

    struct FrameGraphRenderPassCache
    {
        void Init()
        {
            RenderPassMap.clear();
        }
        
        void ShutDown()
        {
            RenderPassMap.clear();
        }

        std::unordered_map<std::string,Ref<FrameGraphRenderPass>> RenderPassMap;
    };
    
    struct FrameGraphResourceCache
    {
        void init()
        {
            ResourceMap.clear();
            Resources.clear();
        };
        
        void ShutDown()
        {
            Context = nullptr;
            ResourceMap.clear();
            Resources.clear();
        };

        VulkanContext* Context;
        
        // 名字到Resource的Index
        std::unordered_map<std::string,uint32> ResourceMap;

        std::vector<Ref<FrameGraphResource>> Resources;
    };

    struct FrameGraphNodeCache
    {
        void Init()
        {
            NameNodeMap.clear();
            Nodes.clear();
        }
        
        void ShutDown()
        {
            VkContext = nullptr;
            NameNodeMap.clear();
            Nodes.clear();
        }

        VulkanContext* VkContext;

        // 名字到FrameGraphNode
        std::unordered_map<std::string,uint32> NameNodeMap;

        std::vector<Ref<FrameGraphNode>> Nodes;
    };

    class FrameGraphBuilder
    {
    public:
        void Init(VulkanContext* Context);
        void ShutDown();

        void RegisterRenderPass(std::string Name,Ref<FrameGraphRenderPass> Renderpass);

        FrameGraphResourceHandle CreateNodeOutput(const FrameGraphResourceOutputCreation& Creation,FrameGraphNodeHandle Producer);
        FrameGraphResourceHandle CreateNodeInput(const FrameGraphResourceInputCreation& Creation);
        FrameGraphNodeHandle CreateNode(const FrameGraphNodeCreation& Creation);

        Ref<FrameGraphNode> GetNode(std::string Name);
        Ref<FrameGraphNode> AccessNode( FrameGraphNodeHandle Handle );
        
        Ref<FrameGraphResource> GetResource(std::string Name);
        Ref<FrameGraphResource> AccessResource(FrameGraphResourceHandle handle);

        FrameGraphResourceCache ResourceCache;
        FrameGraphNodeCache NodeCache;
        FrameGraphRenderPassCache RenderPassCache;
        

        VulkanContext* mContext = nullptr;
        static constexpr uint32            k_max_render_pass_count             = 256;
        static constexpr uint32            k_max_resources_count               = 1024;
        static constexpr uint32            k_max_nodes_count                   = 1024;

        static constexpr cstring        k_name                              = "FrameGraphBuilder";
    };


    /**
     * 
     */
    struct FrameGraph
    {
        void Init(FrameGraphBuilder* Builder);
        void ShutDown();
        
        void EnableRenderPass(cstring RenderPassName);
        void DisableRenderPass(cstring RenderPassName);
        
        void Parse(const char* FilePath);
        void Compile();
        
        void Render(Ref<VulkanCommandBuffer> Commands,Ref<RenderScene> Scene);
        void OnResize(uint32 NewWidth,uint32 NewHeight);

        Ref<FrameGraphNode> GetNode(cstring name);
        Ref<FrameGraphNode> AccessNode(FrameGraphNodeHandle handle);

        Ref<FrameGraphResource> GetResource(cstring name);
        Ref<FrameGraphResource> AccessResource(FrameGraphResourceHandle handle);

        void Render(Ref<VulkanCommandBuffer> CmdBuffer,Ref<VulkanModel> RenderModel);

        void AddNode(FrameGraphNodeCreation& node);

        std::vector<FrameGraphNodeHandle> Nodes;

        FrameGraphBuilder*  builder;

        string  name;
        
    };
}
