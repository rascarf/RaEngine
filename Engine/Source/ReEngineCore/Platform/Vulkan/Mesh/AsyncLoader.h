#pragma once
#include "FileLoadRequest.h"
#include "Core/Timestep.h"
#include "MultiThread/PinnedThread.h"
#include "MultiThread/SimpleMultThreadVector.h"

namespace ReEngine
{
    class AsyncLoader
    {
    public:
        AsyncLoader(){}
        ~AsyncLoader();
    
        void Init(class AsyncLoaderMainTask* InMainTask,Ref<VulkanDevice> InDevice);
        void Update_IOThread();
        void Update_MainThread();
        void ShutDown();

        void RequestTexture(std::string path,FileLoadCallBack CallBack);
        void RequestBufferUpload(void* Data);
        void RequestBufferCopy(Ref<VulkanBuffer> Src,Ref<VulkanBuffer> Dst);

        class AsyncLoaderMainTask*                 OwnerTask = nullptr;
        SimpleMultThreadVector<FileLoadRequest>    FileLoadRequests;
        SimpleMultThreadVector<UploadRequest>      UploadRequests;
        SimpleMultThreadVector<FileReadyRequest>   ReadyUploadRequests;
        
        Ref<VulkanBuffer>                          StagingBuffer = nullptr;
        std::atomic_size_t                         StagingBufferOffset;

        Ref<VulkanBuffer>                          CPUBufferReady = nullptr;
        Ref<VulkanBuffer>                          GPUBufferReady = nullptr;
        Ref<VulkanTexture>                         TextureReady = nullptr;

        std::vector<VkCommandPool>                 CommandPools;
        std::vector<Ref<NewVulkanCommandBuffer>>   CommandBuffer;
        VkSemaphore                                TransferCompleteSemaphore;
        VkFence                                    TransFence;

        Ref<VulkanDevice>                          mDevice;
    };


    // 注意，类里面的所有资源，都是MainThread申请的，生命周期跟随着MainThread
    class AsyncLoaderMainTask : public PinnedThreadTask
    {
    public:

        AsyncLoaderMainTask(PinnedThread* InRunThread,Ref<VulkanDevice> InDevice):
        PinnedThreadTask(InRunThread)
        {
            mTaskScheduler = InRunThread->PinnedThreadLoopTask->mTaskScheduler;
            
            mAsyncLoader = CreateRef<AsyncLoader>();
            mAsyncLoader->Init(this,InDevice);
            
            InRunThread->AddTaskToThread(this);
        }

        virtual void Update_MainThread(Timestep ts)
        {
            mAsyncLoader->Update_MainThread();
        }
    
        virtual void Execute() override
        {
            while(Run)
            {
                mAsyncLoader->Update_IOThread();
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


