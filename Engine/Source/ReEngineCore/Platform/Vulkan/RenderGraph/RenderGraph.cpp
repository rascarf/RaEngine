#include <json.hpp>
#include "RenderGraph.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Resource/AssetManager/AssetManager.h"

namespace ReEngine
{
    static FrameGraphResourceType StringToResourceType(cstring InputType)
    {
        if( strcmp(InputType,"texture") == 0)
        {
            return FrameGraphResourceType::FrameGraphResourceType_Texture;
        }

        if( strcmp(InputType,"attachment") == 0)
        {
            return FrameGraphResourceType::FrameGraphResourceType_Attachment;
        }

        if( strcmp(InputType,"buffer") == 0)
        {
            return FrameGraphResourceType::FrameGraphResourceType_Buffer;
        }

        if( strcmp(InputType,"reference") == 0)
        {
            // 需要拓展出去给下一个用，但是不是新创建的
            return FrameGraphResourceType::FrameGraphResourceType_Reference;
        }

        return FrameGraphResourceType::FrameGraphResourceType_Invalid;
    }

    static VkAttachmentLoadOp PassOperationToVkType(RenderPassOperation Operation)
    {
        switch(Operation)
        {
        case RenderPassOperation::Clear:
            return VK_ATTACHMENT_LOAD_OP_CLEAR;
        case RenderPassOperation::Load:
            return VK_ATTACHMENT_LOAD_OP_LOAD;
        case RenderPassOperation::DontCare:
            return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        default:
            return VK_ATTACHMENT_LOAD_OP_MAX_ENUM;
        }
    }

    static RenderPassOperation StringToRenderPassOperation(cstring op)
    {
        if ( strcmp( op, "VK_ATTACHMENT_LOAD_OP_CLEAR" ) == 0 )
        {
            return RenderPassOperation::Clear;
        }
        else if ( strcmp( op, "VK_ATTACHMENT_LOAD_OP_LOAD" ) == 0 )
        {
            return RenderPassOperation::Load;
        }
        
        return RenderPassOperation::DontCare;
    }

    static void CreateRenderTarget(FrameGraph* framegraph,FrameGraphNode* node)
    {
        std::vector<Ref<VulkanTexture>> ColorAttachments;
        std::vector<VkAttachmentLoadOp> colorLoadAction;
        
        Ref<VulkanTexture> DepthAttachment;
        VkAttachmentLoadOp depthLoadAction;
        
        for(size_t OutIndex = 0; OutIndex < node->Outputs.size();OutIndex++)
        {
            Ref<FrameGraphResource> OutputResource = framegraph->AccessResource(node->Outputs[OutIndex]);

            FrameGraphResourceInfo& Info = OutputResource->ResourceInfo;

            if(OutputResource->ResourceType == FrameGraphResourceType::FrameGraphResourceType_Attachment)
            {
                if(Info.Texture.Format == VK_FORMAT_D32_SFLOAT)
                {
                    DepthAttachment = OutputResource->ResourceInfo.Texture.Texture;
                    depthLoadAction = VK_ATTACHMENT_LOAD_OP_CLEAR;
                }
                else
                {
                    ColorAttachments.emplace_back(OutputResource->ResourceInfo.Texture.Texture);
                    colorLoadAction.push_back(PassOperationToVkType(Info.Texture.LoadOp));
                }
            }
        }

        for(size_t InputIndex = 0 ; InputIndex < node->Inputs.size() ; InputIndex++)
        {
            Ref<FrameGraphResource> InputResource = framegraph->AccessResource(node->Inputs[InputIndex]);

            if(InputResource->ResourceType == FrameGraphResourceType::FrameGraphResourceType_Buffer||
               InputResource->ResourceType == FrameGraphResourceType::FrameGraphResourceType_Reference)
            {
                continue;
            }

            Ref<FrameGraphResource> resource = framegraph->GetResource(InputResource->Name.c_str());
            FrameGraphResourceInfo& info = resource->ResourceInfo;

            InputResource->ResourceInfo.Texture.Texture = info.Texture.Texture;

            if(InputResource->ResourceType == FrameGraphResourceType::FrameGraphResourceType_Texture)
            {
                continue;
            }

            if(info.Texture.Format == VK_FORMAT_D32_SFLOAT)
            {
                DepthAttachment = info.Texture.Texture;
                depthLoadAction = VK_ATTACHMENT_LOAD_OP_LOAD;
            }
            else
            {
                ColorAttachments.emplace_back(info.Texture.Texture);
                colorLoadAction.push_back(VK_ATTACHMENT_LOAD_OP_LOAD);;
            }
        }

        auto device = framegraph->builder->mContext->GetVulkanInstance()->GetDevice();
        if(DepthAttachment)
        {
            VulkanRenderPassInfo PassInfo
            {
                (int32)ColorAttachments.size(),ColorAttachments,colorLoadAction,VK_ATTACHMENT_STORE_OP_STORE,
                DepthAttachment,depthLoadAction,VK_ATTACHMENT_STORE_OP_STORE
            };

            node->RenderTarget = VulkanRenderTarget::Create(
                device,
                PassInfo
            );
        }
        else
        {
            VulkanRenderPassInfo PassInfo
            {
                (int32)ColorAttachments.size(),ColorAttachments,colorLoadAction,VK_ATTACHMENT_STORE_OP_STORE
            };

            VulkanRenderTarget::Create(
                device,
                PassInfo
            );
        }
    }

    static void ComputeEdge(FrameGraph* FrameGraph,Ref<FrameGraphNode> Node,uint32 NodeIndex)
    {
        for(size_t r = 0 ; r < Node->Inputs.size() ; r++)
        {
            Ref<FrameGraphResource> Resource = FrameGraph->AccessResource(Node->Inputs[r]);
            Ref<FrameGraphResource> OutputResource = FrameGraph->GetResource(Resource->Name.c_str());

            if(OutputResource == nullptr && !Resource->ResourceInfo.External)
            {
                RE_CORE_ERROR("{0} is not produced by any node",Resource->Name);
                continue;
            }

            Resource->Producer = OutputResource->Producer;
            Resource->ResourceInfo = OutputResource->ResourceInfo;
            Resource->OutputHandle = OutputResource->OutputHandle;

            Ref<FrameGraphNode> ParentNode = FrameGraph->AccessNode(Resource->Producer);
            RE_CORE_INFO( "{2} [{3}] is rely on {0} [{1}] \n", ParentNode->Name, Resource->Producer.Index, Node->Name, NodeIndex);

            ParentNode->Edges.push_back(FrameGraph->Nodes[NodeIndex]);
        }
    }

    void FrameGraphBuilder::Init(VulkanContext* Context)
    {
        mContext = Context;
        ResourceCache.Context = Context;
        NodeCache.VkContext = Context;

        ResourceCache.init();
        NodeCache.Init();
        RenderPassCache.Init();
        
    }

    void FrameGraphBuilder::ShutDown()
    {
        ResourceCache.ShutDown();
        NodeCache.ShutDown();
        RenderPassCache.ShutDown();
    }

    void FrameGraphBuilder::RegisterRenderPass(std::string Name, Ref<FrameGraphRenderPass> Renderpass)
    {
        if(RenderPassCache.RenderPassMap.contains(Name))
        {
            return;
        }

        if(!NodeCache.NameNodeMap.contains(Name))
        {
            return;
        }

        RenderPassCache.RenderPassMap.insert(std::make_pair(Name,Renderpass));
        
        const auto it = NodeCache.NameNodeMap.find(Name);
        const Ref<FrameGraphNode> Node = NodeCache.Nodes[it->second];
        Node->GraphRenderPass = Renderpass;
    }

    FrameGraphResourceHandle FrameGraphBuilder::CreateNodeOutput(const FrameGraphResourceOutputCreation& Creation,FrameGraphNodeHandle Producer)
    {
        FrameGraphResourceHandle ResourceHandle;
        ResourceHandle.Index = ResourceCache.Resources.size();

        Ref<FrameGraphResource> Resource = CreateRef<FrameGraphResource>();

        Resource->Name = Creation.Name;
        Resource->ResourceType = Creation.ResourceType;

        if(Creation.ResourceType != FrameGraphResourceType::FrameGraphResourceType_Reference)
        {
            Resource->ResourceInfo = Creation.ResourceInfo;
            Resource->OutputHandle = ResourceHandle;
            Resource->Producer = Producer;
            Resource->RefCount = 0;

            ResourceCache.ResourceMap.emplace(std::make_pair(Resource->Name.c_str(),ResourceHandle.Index));
        }
        
        ResourceCache.Resources.push_back(Resource);

        return ResourceHandle;
    }

    FrameGraphResourceHandle FrameGraphBuilder::CreateNodeInput(const FrameGraphResourceInputCreation& Creation)
    {
        FrameGraphResourceHandle ResourceHandle;
        ResourceHandle.Index = ResourceCache.Resources.size();

        Ref<FrameGraphResource> Resource = CreateRef<FrameGraphResource>();
        Resource->ResourceInfo = {};
        Resource->Producer.Index = k_invalid_index;
        Resource->OutputHandle.Index = k_invalid_index;

        Resource->ResourceType = Creation.ResourceType;
        Resource->Name = Creation.Name;
        Resource->RefCount = 0;

        ResourceCache.Resources.push_back(Resource);
        
        return ResourceHandle;
    }

    FrameGraphNodeHandle FrameGraphBuilder::CreateNode(const FrameGraphNodeCreation& Creation)
    {
        FrameGraphNodeHandle NodeHandle;
        NodeHandle.Index = NodeCache.Nodes.size();

        Ref<FrameGraphNode> Node = CreateRef<FrameGraphNode>();
        Node->Name = Creation.Name;
        Node->Enabled = Creation.Enabled;

        Node->Inputs.reserve(Creation.Inputs.size());
        Node->Outputs.reserve(Creation.Outputs.size());
        Node->Edges.reserve(Creation.Outputs.size());

        Node->RenderTarget.reset();

        NodeCache.Nodes.push_back(Node);
        
        NodeCache.NameNodeMap.insert(std::make_pair(Node->Name.c_str(), NodeHandle.Index));

        for(size_t i = 0; i < Creation.Outputs.size(); ++i)
        {
            const FrameGraphResourceOutputCreation& OutputCreation = Creation.Outputs[i];

            FrameGraphResourceHandle output = CreateNodeOutput(OutputCreation,NodeHandle);

            Node->Outputs.push_back(output);
        }

        for(size_t i = 0 ; i < Creation.Inputs.size(); ++i)
        {
            const FrameGraphResourceInputCreation& inputCreation = Creation.Inputs[i];

            FrameGraphResourceHandle InputHandle = CreateNodeInput(inputCreation);

            Node->Inputs.push_back(InputHandle);
        }

        return NodeHandle;
    }

    Ref<FrameGraphNode> FrameGraphBuilder::GetNode(std::string Name)
    {
        const auto NodeHandle = NodeCache.NameNodeMap.find(Name);

        return NodeCache.Nodes[NodeHandle->second];
    }

    Ref<FrameGraphNode> FrameGraphBuilder::AccessNode(FrameGraphNodeHandle Handle)
    {
        return  NodeCache.Nodes[Handle.Index];
    }

    Ref<FrameGraphResource> FrameGraphBuilder::GetResource(std::string Name)
    {
        const auto NodeHandle = ResourceCache.ResourceMap.find(Name);

        return ResourceCache.Resources[NodeHandle->second];
    }

    Ref<FrameGraphResource> FrameGraphBuilder::AccessResource(FrameGraphResourceHandle handle)
    {
        return  ResourceCache.Resources[handle.Index];
    }


    //---------------------------------------------------------

    void FrameGraph::Init(FrameGraphBuilder* Builder)
    {
        builder = Builder;
    }

    void FrameGraph::ShutDown()
    {
        for(uint32 i = 0; i < Nodes.size();i++)
        {
            FrameGraphNodeHandle Handle = Nodes[i];
            Ref<FrameGraphNode> Node = builder->AccessNode(Handle);

            Node->RenderTarget.reset();
            
            Node->Inputs.clear();
            Node->Outputs.clear();
            Node->Edges.clear();
        }

        Nodes.clear();
    }

    void FrameGraph::EnableRenderPass(cstring RenderPassName)
    {
        Ref<FrameGraphNode> Node = builder->GetNode(RenderPassName);
        Node->Enabled = true;
    }

    void FrameGraph::DisableRenderPass(cstring RenderPassName)
    {
        Ref<FrameGraphNode> Node = builder->GetNode(RenderPassName);
        Node->Enabled = false;
    }

    void FrameGraph::Parse(const char* FilePath)
    {
        using json = nlohmann::json;
        using string = std::string;

        uint32 dataSize = 0;
        uint8* dataPtr  = nullptr;
        AssetManager::ReadFile(FilePath, dataPtr, dataSize);

        const json GraphData = json::parse(dataPtr);
        string NameValue = GraphData.value("name","");
        name = NameValue.c_str();

        json Passes = GraphData[ "passes" ];
        for(size_t i = 0 ; i < Passes.size() ; ++i)
        {
            json Pass = Passes[ i ];

            json PassInputs = Pass["inputs"];
            json PassOutputs = Pass["outputs"];

            FrameGraphNodeCreation NodeCreation{};
            NodeCreation.Inputs.reserve(PassInputs.size());
            NodeCreation.Outputs.reserve(PassOutputs.size());
            
            for(size_t PassInputIndex = 0 ; PassInputIndex < PassInputs.size();PassInputIndex++)
            {
                json PassInput = PassInputs[PassInputIndex];

                FrameGraphResourceInputCreation InputCreation = {};
                string InputType = PassInput.value("type","");
                assert(!InputType.empty());

                string InputName = PassInput.value("name","");
                assert(!InputName.empty());

                InputCreation.ResourceType = StringToResourceType(InputType.c_str());
                InputCreation.ResourceInfo.External = false;
                InputCreation.Name = InputName.c_str();
            
                NodeCreation.Inputs.push_back(InputCreation);
            }

            
            for(size_t PassOutputIndex = 0 ; PassOutputIndex < PassOutputs.size(); PassOutputIndex++)
            {
                json PassOutput = PassOutputs[PassOutputIndex];

                FrameGraphResourceOutputCreation OutputCreation{};
                string OutputType = PassOutput.value("type","");
                string OutputName = PassOutput.value("name","");

                OutputCreation.ResourceType = StringToResourceType(OutputType.c_str());
                OutputCreation.Name = OutputName.c_str();

                switch (OutputCreation.ResourceType)
                {
                    case FrameGraphResourceType::FrameGraphResourceType_Attachment:
                    case FrameGraphResourceType::FrameGraphResourceType_Texture:
                    {
                        string format = PassOutput.value("format","");
                        OutputCreation.ResourceInfo.Texture.Format = util_string_to_vk_format(format.c_str());

                        string LoadOp = PassOutput.value("op","");

                        OutputCreation.ResourceInfo.Texture.LoadOp = StringToRenderPassOperation(LoadOp.c_str());
                        
                        json Resolution = PassOutput["resolution"];

                        OutputCreation.ResourceInfo.Texture.Width = Resolution[0];
                        OutputCreation.ResourceInfo.Texture.Height = Resolution[1];
                        OutputCreation.ResourceInfo.Texture.Depth = 1;
                    }break;

                case FrameGraphResourceType::FrameGraphResourceType_Buffer:
                    {
                        
                    }break;
                }

                NodeCreation.Outputs.push_back(OutputCreation);
            }

            NameValue = Pass.value("name","");

            NodeCreation.Name = NameValue.c_str();
            NodeCreation.Enabled = Pass.value("enable",true);

            FrameGraphNodeHandle NodeHanlde = builder->CreateNode(NodeCreation);
            Nodes.push_back(NodeHanlde);
        }
    }

    void FrameGraph::Compile()
    {
        // 检查input是不是被其他的node创造了
        // 将不可达的node剔除掉

        for(size_t i = 0 ; i < Nodes.size() ; i++)
        {
            Ref<FrameGraphNode> node = builder->AccessNode(Nodes[i]);

            node->Edges.clear();
        }

        for(size_t i = 0 ; i < Nodes.size() ; i++)
        {
            Ref<FrameGraphNode> Node = builder->AccessNode(Nodes[i]);
            if(!Node->Enabled)
            {
                continue;
            }

            ComputeEdge(this,Node,i);
        }

        // 拓扑排序
        std::vector<FrameGraphNodeHandle> SortedNodes;
        std::vector<FrameGraphNodeHandle> Stack;
        std::vector<uint8> Visited(Nodes.size(),0);
        
        for(uint32 n = 0 ; n < Nodes.size(); n++)
        {
            Ref<FrameGraphNode> Node = builder->AccessNode(Nodes[n]);
            if(!Node->Enabled)
            {
                continue;
            }

            Stack.push_back(Nodes[n]);

            while(Stack.size() > 0)
            {
                FrameGraphNodeHandle NodeHandle = Stack.back();

                if(Visited[NodeHandle.Index] == 2)
                {
                    Stack.pop_back();
                    continue;
                }

                if(Visited[NodeHandle.Index] == 1 )
                {
                    Visited[NodeHandle.Index] = 2;

                    SortedNodes.push_back(NodeHandle);

                    Stack.pop_back();

                    continue;
                }

                Visited[NodeHandle.Index] = 1;

                Ref<FrameGraphNode> Node = builder->AccessNode(NodeHandle);

                // 叶子节点
                if(Node->Edges.size() == 0)
                {
                    continue;
                }

                for(size_t r = 0 ; r < Node->Edges.size() ; r++)
                {
                    FrameGraphNodeHandle ChildHandle = Node->Edges[r];

                    if(!Visited[ChildHandle.Index])
                    {
                        Stack.push_back(ChildHandle);
                    }
                }
            }
        }

        
        Nodes.clear();
        for(int i = SortedNodes.size() - 1 ; i >= 0 ; --i)
        {
            Nodes.push_back(SortedNodes[i]);
        }
        
        // 此时Nodes必定满足拓扑排序
        // 也就是i <= j,那么节点i必定先于j出现
        size_t ResourceCount = builder->ResourceCache.Resources.size();
        std::vector<FrameGraphNodeHandle> Allocations{ResourceCount};
        for(auto& Handle : Allocations)
        {
            Handle.Index = k_invalid_index;
        }
        
        std::vector<FrameGraphNodeHandle> Deallocations{ResourceCount};
        for(auto& Handle : Deallocations)
        {
            Handle.Index = k_invalid_index;
        }
        
        std::vector<Ref<VulkanTexture>> FreeList;

        // ------ free_list
        for(uint32_t i = 0 ; i < Nodes.size();i++)
        {
            Ref<FrameGraphNode> Node =  builder->AccessNode(Nodes[i]);
            if(!Node->Enabled)
            {
                continue;
            }

            for(uint32 InputIndex = 0 ; InputIndex < Node->Inputs.size() ; InputIndex++)
            {
                Ref<FrameGraphResource> InputResource = builder->AccessResource(Node->Inputs[InputIndex]);
                Ref<FrameGraphResource> Resource = builder->AccessResource(InputResource->OutputHandle);

                Resource->RefCount ++;
            }
        }

        for(uint32 i = 0 ; i < Nodes.size(); i++)
        {
            Ref<FrameGraphNode> Node = builder->AccessNode(Nodes[i]);
            if(!Node->Enabled)
            {
                continue;
            }

            for(uint32 OutputIndex = 0 ; OutputIndex < Node->Outputs.size() ; OutputIndex ++)
            {
                uint32 ResourceIndex = Node->Outputs[OutputIndex].Index;
                Ref<FrameGraphResource> Resource = builder->AccessResource(Node->Outputs[OutputIndex]);

                if(!Resource->ResourceInfo.External && Allocations[ResourceIndex].Index == k_invalid_index)
                {
                    Allocations[ResourceIndex] = Nodes[ i ];

                    if(Resource->ResourceType == FrameGraphResourceType::FrameGraphResourceType_Attachment)
                    {
                        FrameGraphResourceInfo& info = Resource->ResourceInfo;
                        bool bIsDepth = info.Texture.Format == VK_FORMAT_D32_SFLOAT;

                        Ref<VulkanTexture> NewTexture;
                        
                        if(FreeList.size() > 0)
                        {
                            //Find Best Fit
                            Ref<VulkanTexture> AliasTexture = FreeList.back();
                            FreeList.pop_back();

                            NewTexture = VulkanTexture::CreateFrameGraphAliasingTexture(
                                builder->mContext->GetVulkanInstance()->GetDevice(),
                                AliasTexture,
                                info.Texture.Format,
                                info.Texture.Width,
                                info.Texture.Height,
                                bIsDepth? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                1
                            );
                        }

                        if(!NewTexture)
                        {
                            NewTexture = VulkanTexture::CreateFrameGraphTexture(
                                builder->mContext->GetVulkanInstance()->GetDevice(),
                                info.Texture.Format,
                                info.Texture.Width,
                                info.Texture.Height,
                                bIsDepth? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                1
                            );
                        }
                        
                        info.Texture.Texture = NewTexture;
                    }
                }
            }

            for(uint32_t InputIndex = 0 ; InputIndex < Node->Inputs.size() ; InputIndex++)
            {
                // compile到这个节点的时候，这个节点所使用的上一个节点的Output对应的资源引用计数减少
                Ref<FrameGraphResource> InputResource = builder->AccessResource(Node->Inputs[InputIndex]);
                Ref<FrameGraphResource> OriginalResource = builder->AccessResource(InputResource->OutputHandle);

                Deallocations[InputResource->OutputHandle.Index] = Nodes[i];
                
                OriginalResource->RefCount--;
                
                if(!OriginalResource->ResourceInfo.External && OriginalResource->RefCount == 0)
                {
                    if(OriginalResource->ResourceType == FrameGraphResourceType::FrameGraphResourceType_Attachment || OriginalResource->ResourceType == FrameGraphResourceType::FrameGraphResourceType_Texture)
                    {
                        FreeList.push_back(OriginalResource->ResourceInfo.Texture.Texture);
                    }
                }
            }
        }

        Allocations.clear();
        Deallocations.clear();
        FreeList.clear();

        for(uint32 NodeIndex = 0 ; NodeIndex < Nodes.size();NodeIndex++)
        {
            Ref<FrameGraphNode> Node = builder->AccessNode(Nodes[NodeIndex]);
            if(!Node->Enabled)
            {
                continue;
            }

            if(!Node->RenderTarget)
            {
                CreateRenderTarget(this,Node.get());
            }
        }
    }

    void FrameGraph::Render(Ref<VulkanCommandBuffer> Commands, Ref<RenderScene> Scene)
    {
    }

    void FrameGraph::OnResize(uint32 NewWidth, uint32 NewHeight)
    {
    }

    void FrameGraph::Render(Ref<VulkanCommandBuffer> CmdBuffer, Ref<VulkanModel> RenderModel)
    {
        for(uint32 n = 0 ; n < Nodes.size() ; n++)
        {
            Ref<FrameGraphNode> Node = builder->AccessNode(Nodes[n]);
            if(!Node->Enabled)
            {
                continue;
            }

            for(uint32 i = 0 ; i < Node->Inputs.size() ; i++)
            {
                Ref<FrameGraphResource> Resource = builder->AccessResource(Node->Inputs[i]);

                if(Resource->ResourceType == FrameGraphResourceType::FrameGraphResourceType_Texture)
                {
                    Ref<VulkanTexture> Texture = Resource->ResourceInfo.Texture.Texture;
                    bool bIsDepth = Texture->Format == VK_FORMAT_D32_SFLOAT;
                    ImageLayoutBarrier Src = bIsDepth ? ImageLayoutBarrier::DepthStencilAttachment : ImageLayoutBarrier::ColorAttachment;
                    ImageLayoutBarrier Dst = ImageLayoutBarrier::PixelShaderRead;

                    ImagePipelineBarrier(CmdBuffer->CmdBuffer,Texture->Image,Src,Dst,0,1,bIsDepth);
                }
            }

            for(uint32 o = 0 ; o < Node->Outputs.size() ; o++)
            {
                Ref<FrameGraphResource> Resource = builder->AccessResource(Node->Outputs[ o ]);

                if(Resource->ResourceType == FrameGraphResourceType::FrameGraphResourceType_Attachment)
                {
                    Ref<VulkanTexture> Texture = Resource->ResourceInfo.Texture.Texture;
                    bool bIsDepth = Texture->Format == VK_FORMAT_D32_SFLOAT;
                    ImageLayoutBarrier Src = ImageLayoutBarrier::Undefined;
                    ImageLayoutBarrier Dst = bIsDepth ? ImageLayoutBarrier::DepthStencilAttachment : ImageLayoutBarrier::ColorAttachment;

                    ImagePipelineBarrier(CmdBuffer->CmdBuffer,Texture->Image,Src,Dst,0,1,bIsDepth);
                }
            }

            Node->GraphRenderPass->PreRender(CmdBuffer,RenderModel);

            Node->RenderTarget->BeginRenderPass(CmdBuffer->CmdBuffer);
            
            Node->GraphRenderPass->Render(CmdBuffer,RenderModel);

            Node->RenderTarget->EndRenderPass(CmdBuffer->CmdBuffer);
            
        }
    }

    void FrameGraph::AddNode(FrameGraphNodeCreation& node)
    {
    }

    Ref<FrameGraphNode> FrameGraph::GetNode(cstring name)
    {
        return builder->GetNode(name);
    }

    Ref<FrameGraphNode> FrameGraph::AccessNode(FrameGraphNodeHandle handle)
    {
        return builder->AccessNode(handle);
    }

    Ref<FrameGraphResource> FrameGraph::GetResource(cstring name)
    {
        return builder->GetResource(name);
    }

    Ref<FrameGraphResource> FrameGraph::AccessResource(FrameGraphResourceHandle handle)
    {
        return builder->AccessResource(handle);
    }
}

