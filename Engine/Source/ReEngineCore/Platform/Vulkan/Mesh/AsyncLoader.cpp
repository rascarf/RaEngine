#include "AsyncLoader.h"

#include "cxxtimer.hpp"
#include "ImageLoader.h"
#include "Core/Application.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Resource/AssetManager/AssetManager.h"

namespace ReEngine
{
    AsyncLoader::~AsyncLoader()
    {
        ShutDown();
    }

    void AsyncLoader::Init(AsyncLoaderMainTask* InMainTask,Ref<VulkanDevice> InDevice)
    {
        OwnerTask = InMainTask;
        mDevice = InDevice;

        StagingBuffer = VulkanBuffer::CreateBuffer(
            InDevice,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            64 * 1024 * 1024 //64MB
        );

        for(uint32_t i = 0 ; i < RE_BUFFERCOUNT ; ++i)
        {
            VkCommandPool NewCommandPool; 
            VkCommandPoolCreateInfo cmd_pool_info = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr };
            cmd_pool_info.queueFamilyIndex = mDevice->GetTransferQueue()->GetFamilyIndex();
            cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            vkCreateCommandPool(mDevice->GetInstanceHandle(),&cmd_pool_info,nullptr,&NewCommandPool);
            CommandPools.emplace_back(NewCommandPool);
            
            Ref<NewVulkanCommandBuffer> CmdBuffer = NewVulkanCommandBuffer::Create(mDevice,CommandPools[i],VK_COMMAND_BUFFER_LEVEL_PRIMARY,mDevice->GetTransferQueue());
            CommandBuffer.emplace_back(CmdBuffer);
        }

        VkSemaphoreCreateInfo semaphore_info{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        vkCreateSemaphore( mDevice->GetInstanceHandle(), &semaphore_info, nullptr, &TransferCompleteSemaphore );

        VkFenceCreateInfo fence_info{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(  mDevice->GetInstanceHandle(), &fence_info, nullptr, &TransFence );
        
    }

    void AsyncLoader::Update_IOThread()
    {
        if(UploadRequests.GetSize() > 0 )
        {
            if(vkGetFenceStatus(mDevice->GetInstanceHandle(),TransFence) != VK_SUCCESS)
            {
                return ;
            }

            vkResetFences(mDevice->GetInstanceHandle(),1 , &TransFence);
            
            UploadRequest CurrentLoad = UploadRequests.Back();
            UploadRequests.Pop_Back();
            
            Ref<NewVulkanCommandBuffer> CurrtBuffer = CommandBuffer[mDevice->mContext->GetCurrtIndex()];
            CurrtBuffer->Begin();

            if(CurrentLoad.Texture && !CurrentLoad.Texture->IsUpload)
            {
                const uint32 k_texture_channels = 4;
                const uint32 k_texture_alignment = 4;

                const uint32_t AlignedImageSize = MemoryAlign(CurrentLoad.Texture->Width * CurrentLoad.Texture->Height * k_texture_channels , k_texture_alignment);
                const uint32_t current_offset = std::atomic_fetch_add( &StagingBufferOffset, AlignedImageSize );

                StagingBuffer->Map();
                StagingBuffer->CopyFrom((void*)CurrentLoad.data,AlignedImageSize);
                StagingBuffer->UnMap();

                CurrtBuffer->UploadTextureData(CurrentLoad.Texture,CurrentLoad.data,StagingBuffer,current_offset);

                delete CurrentLoad.data;
            }
            CurrtBuffer->End();

            VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &CurrtBuffer->CmdBuffer;
            
            VkPipelineStageFlags wait_flag[] { VK_PIPELINE_STAGE_TRANSFER_BIT };
            VkSemaphore wait_semaphore[] { TransferCompleteSemaphore };
            submitInfo.pWaitSemaphores = wait_semaphore;
            submitInfo.pWaitDstStageMask = wait_flag;

            Ref<VulkanQueue> used_queue = mDevice->GetTransferQueue();
            vkQueueSubmit( used_queue->GetHandle(), 1, &submitInfo, TransFence );

            FileReadyRequest NewReady;
            NewReady.Callback = std::move(CurrentLoad.Callback);
            NewReady.Path = std::move(CurrentLoad.Path);
            NewReady.BufferHandle = std::move(CurrentLoad.GpuBuffer);
            NewReady.TextureHanle = std::move(CurrentLoad.Texture);

            ReadyUploadRequests.Emplace_Back(NewReady);
        }
        
       if(FileLoadRequests.GetSize() > 0)
       {
           auto FileLoadRequest = FileLoadRequests.Back();
           FileLoadRequests.Pop_Back();

           cxxtimer::Timer LoadTimer;
           LoadTimer.start();

           uint32 DataSize = 0;
           uint8* DataPtr = nullptr;

           if(!AssetManager::ReadFile(FileLoadRequest.Path,DataPtr,DataSize))
           {
               RE_CORE_ERROR("Failed to Load Image : {0}",FileLoadRequest.Path.c_str());
           }

           int32 comp   = 0;
           int32 width  = 0;
           int32 height = 0;
           uint8* rgbaData = StbImage::LoadFromMemory(DataPtr, DataSize, &width, &height, &comp, 4);

           delete[] DataPtr;
           DataPtr = nullptr;

           if (rgbaData == nullptr)
           {
               RE_CORE_ERROR("Failed load image From StbImage: {0}", FileLoadRequest.Path.c_str());
               return;
           }

          if(rgbaData)
          {
              LoadTimer.stop();
              RE_INFO("File {0} Read in {1} ms",FileLoadRequest.Path,LoadTimer.count());
          }
           
           UploadRequest NewUploadRequest;
           NewUploadRequest.Path = FileLoadRequest.Path;
           NewUploadRequest.Callback = FileLoadRequest.Callback;
           NewUploadRequest.data = rgbaData;
           NewUploadRequest.Texture = VulkanTexture::Create2D(mDevice,nullptr,VK_FORMAT_R8G8B8A8_UNORM ,VK_IMAGE_ASPECT_COLOR_BIT,width,height,VK_IMAGE_USAGE_SAMPLED_BIT|VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT,VK_SAMPLE_COUNT_1_BIT,ImageLayoutBarrier::PixelShaderRead);
           UploadRequests.Emplace_Back(NewUploadRequest);
       }
    }

    void AsyncLoader::Update_MainThread()
    {
        while(ReadyUploadRequests.GetSize() > 0)
        {
            const auto Ready =  ReadyUploadRequests.Back();
            FileLoadCallBackParamStruct CallBackStruct;
            CallBackStruct.BufferHanle = Ready.BufferHandle;
            CallBackStruct.TextureHanle = Ready.TextureHanle;
            CallBackStruct.FilePath = Ready.Path;
            
            Ready.Callback(CallBackStruct);

            ReadyUploadRequests.Pop_Back();
        }
    }

    void AsyncLoader::ShutDown()
    {
        StagingBuffer.reset();
        for ( uint32_t i = 0; i < RE_BUFFERCOUNT; ++i ) {
            vkDestroyCommandPool( mDevice->GetInstanceHandle(), CommandPools[ i ], nullptr);
        }

        vkDestroySemaphore( mDevice->GetInstanceHandle(), TransferCompleteSemaphore, nullptr );
        vkDestroyFence( mDevice->GetInstanceHandle(), TransFence, nullptr );
        
        mDevice.reset();
    }

    void AsyncLoader::RequestTexture(std::string path,FileLoadCallBack CallbBack)
    {
        FileLoadRequest Request;
        Request.Path = std::move(path);
        Request.Callback = std::move(CallbBack);

        FileLoadRequests.Emplace_Back(Request);
    }

    void AsyncLoader::RequestBufferUpload(void* Data)
    {
        UploadRequest NewUploadRequest;
        NewUploadRequest.data = Data;
        // NewUploadRequest.Callback = CallBack;

        UploadRequests.Emplace_Back(NewUploadRequest);
    }

    void AsyncLoader::RequestBufferCopy(Ref<VulkanBuffer> Src, Ref<VulkanBuffer> Dst)
    {
        UploadRequest NewUploadRequest;
        NewUploadRequest.data = nullptr;
        NewUploadRequest.CpuBuffer = Src;
        NewUploadRequest.GpuBuffer = Dst;
        // NewUploadRequest.Callback = CallBack;

        UploadRequests.Emplace_Back(NewUploadRequest);
    }
    
}

