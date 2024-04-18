#pragma once
#include "SpinLock.hpp"
#include <vector>

namespace ReEngine
{
    template<typename ElementType>
    class SimpleMultThreadVector
    {
    public:

        SpinLock* mlock;
        std::vector<ElementType> Data;
        
        SimpleMultThreadVector(const SimpleMultThreadVector &other) = delete;
        SimpleMultThreadVector(SimpleMultThreadVector &&other) = delete;
        SimpleMultThreadVector &operator= (const SimpleMultThreadVector &other) = delete;
        SimpleMultThreadVector &operator= (SimpleMultThreadVector &&other) = delete;
        
        SimpleMultThreadVector()
        {
            mlock = new SpinLock();
        }
        
        ~SimpleMultThreadVector()
        {
            delete mlock;
        }

        void Emplace_Back(ElementType&& Element)
        {
            LockGuard(mlock);
            
            Data.emplace_back(std::forward<ElementType>(Element));
        }

        void Pop_Back()
        {
            LockGuard(mlock);
            Data.pop_back();
        }

        ElementType& Back()
        {
            LockGuard(mlock);
            return Data.back();
        }
    };
}

