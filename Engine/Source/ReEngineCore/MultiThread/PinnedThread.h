#include "ReEngineCore/Core/Core.h"
#include "enkiTS/TaskScheduler.h"

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
            mTaskScheduler->WaitForNewPinnedTasks();
            
            RE_INFO("New Task Arrived wake up");
            mTaskScheduler->RunPinnedTasks();
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

    PinnedThread(enki::TaskScheduler* InTaskScheduler,int InThreadID);
    ~PinnedThread();
};

struct RunnableTask : public enki::IPinnedTask
{
public:
    PinnedThread* mRunThread;
    
    RunnableTask(PinnedThread* InRunThread)
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
};


