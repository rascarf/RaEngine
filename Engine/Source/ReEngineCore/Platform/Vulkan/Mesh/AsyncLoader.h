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

        void RequestTexture(std::string path);
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
            mTaskScheduler = InRunThread->PinnedThreadLoopTask->mTaskScheduler;
            
            mAsyncLoader = CreateRef<AsyncLoader>();
            mAsyncLoader->Init(this);
            
            InRunThread->AddTaskToThread(this);
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
            
            mTaskScheduler->WaitforTaskSet(this);
            RE_INFO("AsyncLoaderMainTask ShutDown");

            mAsyncLoader.reset();
        }

        virtual ~AsyncLoaderMainTask()
        {
            
        }
        
        Ref<AsyncLoader>                mAsyncLoader = nullptr;
        enki::TaskScheduler*            mTaskScheduler = nullptr;
        std::atomic<bool>               Run = true;
    };
}


