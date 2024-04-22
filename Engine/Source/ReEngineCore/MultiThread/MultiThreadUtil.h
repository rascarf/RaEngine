#pragma once
#include "SimpleMultThreadVector.h"
#include "Core/Core.h"
#include "TaskScheduler.h"
#include "Core/Application.h"

namespace  ReEngine
{
    template<typename ElementType>
    class PARALLEL_TASK : public enki::ITaskSet
    {
    public:
        void ExecuteRange(enki::TaskSetPartition range_, uint32_t ThreadID) override
        {
            for (uint32_t i = range_.start; i < range_.end; ++i)
            {
                // Do work
                Func((*Data)[i],ThreadID);
            }
        }

        PARALLEL_TASK(uint32_t Size, uint32_t MinBatchSize , SimpleMultThreadVector<ElementType>* Data,const std::function<void(ElementType&,uint32_t)>& InFunc)
            : enki::ITaskSet(Size, MinBatchSize),
              Data(Data), Func(InFunc){}

    public:
        SimpleMultThreadVector<ElementType>* Data;
        std::function<void(ElementType&,uint32_t)> Func;
    };

    
    class MultiThreadUtil
    {
    public:
        // 派发的线程数是Data.size / MinBatchSize
        // 每个线程最少处理MinBatchSize个任务
        template<typename ElementType>
        static void PARALLELFOR(SimpleMultThreadVector<ElementType>& Data,std::function<void(ElementType&,uint32_t)> Func, uint32_t MinBatchSize = 1,bool bWait = true)
        {
            PARALLEL_TASK<ElementType> Task(Data.GetSize(),MinBatchSize,&Data,Func);
            enki::TaskScheduler* Scheduler = Application::GetInstance().GetTaskSheduler();
            Scheduler->AddTaskSetToPipe(&Task);

            if(bWait)
            {
                Scheduler->WaitforTaskSet(&Task);
            }
        }
    };
    
}

