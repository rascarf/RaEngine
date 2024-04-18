#pragma once
#include "FileLoadRequest.h"
#include "MultiThread/PinnedThread.h"
#include "MultiThread/SimpleMultThreadVector.h"

namespace ReEngine
{
    class AsyncLoader
    {
    public:
        AsyncLoader(){}
        ~AsyncLoader();
    
        void Init(class AsyncLoaderMainTask* InMainTask);
        void Update();
        void ShutDown();

        void RequestTexture();
        void RequestBufferUpload();
        void RequestBufferCopy();

        class AsyncLoaderMainTask*        OwnerTask = nullptr;
        SimpleMultThreadVector<FileLoadRequest>    FileLoadRequests;
        SimpleMultThreadVector<UploadRequest>      UploadRequests;

        Ref<VulkanBuffer>               StagingBuffer;
        std::atomic_size_t              StagingBufferOffset;

        Ref<VulkanBuffer>               CPUBufferReady;
        Ref<VulkanBuffer>               GPUBufferReady;

        VkCommandPool                   CommandPools[RE_BUFFERCOUNT];
        VkCommandBuffer                 CommandBuffer[RE_BUFFERCOUNT];
        VkSemaphore                     TransferCompleteSemaphore;
        VkFence                     TransFence;
    };


    // 注意，类里面的所有资源，都是MainThread申请的，生命周期跟随着MainThread
    class AsyncLoaderMainTask : public PinnedThreadTask
    {
    public:

        AsyncLoaderMainTask(PinnedThread* InRunThread):
        PinnedThreadTask(InRunThread)
        {
            mAsyncLoader = CreateRef<AsyncLoader>();
            mAsyncLoader->Init(this);
        }
    
        virtual void Execute() override
        {
            while(Run)
            {
                mAsyncLoader->Update();
            }
        }

        virtual void ShutDown()
        {
            Run = false;
            while(!GetIsComplete())
            {
                
            }

            mAsyncLoader.reset();
        }

        ~AsyncLoaderMainTask()
        {
            
        }
        
        Ref<AsyncLoader>                mAsyncLoader = nullptr;
        std::atomic<bool>               Run = true;
    };
}


