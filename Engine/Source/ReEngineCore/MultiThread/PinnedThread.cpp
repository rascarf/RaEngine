#include "PinnedThread.h"

namespace ReEngine
{
    PinnedThread::PinnedThread(enki::TaskScheduler* InTaskScheduler,int InThreadID)
    {
        ThreadID = InThreadID;
        PinnedThreadLoopTask = new PinnedImpl(InThreadID,InTaskScheduler);
    
        InTaskScheduler->AddPinnedTask(PinnedThreadLoopTask);
    }

    PinnedThread::~PinnedThread()
    {
        if(PinnedThreadLoopTask)
        {
            PinnedThreadLoopTask->bExecute = false;
            
            SignalThread();
            
            delete PinnedThreadLoopTask;
        }
    }

    void PinnedThread::AddTaskToThread(enki::IPinnedTask* Task)
    {
        if(PinnedThreadLoopTask->mTaskScheduler)
        {
            PinnedThreadLoopTask->mTaskScheduler->AddPinnedTask(Task);
        }
    }

    void PinnedThread::SignalThread()
    {
        PinnedThreadTask EmptyTask(this);
        AddTaskToThread(&EmptyTask);

        PinnedThreadLoopTask->mTaskScheduler->WaitforTask(PinnedThreadLoopTask);
    }
}

