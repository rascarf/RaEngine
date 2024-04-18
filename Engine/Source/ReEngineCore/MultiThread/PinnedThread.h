#include "ReEngineCore/Core/Core.h"
#include "enkiTS/TaskScheduler.h"

namespace ReEngine
{
    // 绑定在对应序号的特殊线程，绑定了之后就只会运行指定的Task
    // 这是必须的步骤，因为thread被创建出来，我们只能通过Task的方式去与Thread交互
    // 当然 我们可以直接在下面的循环里面把所有的操作都写完，但是这样的话就失去了拓展性
    struct PinnedImpl: public enki::IPinnedTask
    {
    public:
        void Execute() override
        {
            while(mTaskScheduler->GetIsRunning() && bExecute)
            {
                // 如果没有新的PinnedTask到这个线程，那么直接sleep
                // 这个需要我们在析构的时候触发一个新的任务来跳出循环
                mTaskScheduler->WaitForNewPinnedTasks();
            
                mTaskScheduler->RunPinnedTasks();
                RE_INFO("New Task Arrived wake up");
            }
        }
        
        bool bExecute = true;
        enki::TaskScheduler* mTaskScheduler;

        PinnedImpl(uint32_t ID,enki::TaskScheduler* InTaskScheduler)
            :IPinnedTask(ID)
        {
            mTaskScheduler = InTaskScheduler;
        }
    };

    // 分配的线程
    struct PinnedThread 
    {
    public:
        uint32_t ThreadID = 0;

        PinnedImpl* PinnedThreadLoopTask;
        virtual uint32_t GetThreadID()
        {
            return ThreadID;
        }

        virtual void AddTaskToThread(enki::IPinnedTask* Task);
        virtual void SignalThread();

        PinnedThread(enki::TaskScheduler* InTaskScheduler,int InThreadID);
        virtual ~PinnedThread();
    };

    // 运行在绑定的线程上的任务
    struct PinnedThreadTask : public enki::IPinnedTask
    {
    public:
        PinnedThread* mRunThread;
    
        PinnedThreadTask(PinnedThread* InRunThread)
            :IPinnedTask(InRunThread->ThreadID)
        {
            mRunThread = InRunThread;
        }

        virtual void Run()
        {
            mRunThread->AddTaskToThread(this);
        }

        virtual void Execute() override
        {
            RE_INFO("RunnableTask : Current Thread is {0}",mRunThread->ThreadID);
        }

        virtual ~PinnedThreadTask(){}
    };

    struct EmptyThreadTask : public PinnedThreadTask
    {
        virtual void Execute() override{}
        EmptyThreadTask(PinnedThread* InRunThread):PinnedThreadTask(InRunThread){}
    };
}



