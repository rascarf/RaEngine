#include "PinnedThread.h"

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
