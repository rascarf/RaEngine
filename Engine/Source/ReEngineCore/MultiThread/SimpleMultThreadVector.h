#pragma once
#include "SpinLock.hpp"
#include <vector>

namespace ReEngine
{
    template<typename ElementType>
    class SimpleMultThreadVector
    {
    private:
        SpinLock* mlock;
        std::vector<ElementType> Data;
        
        SimpleMultThreadVector(const SimpleMultThreadVector &other) = delete;
        SimpleMultThreadVector(SimpleMultThreadVector &&other) = delete;
        SimpleMultThreadVector &operator= (const SimpleMultThreadVector &other) = delete;
        SimpleMultThreadVector &operator= (SimpleMultThreadVector &&other) = delete;
        
    public:
        SimpleMultThreadVector()
        {
            Data.empty();
            mlock = new SpinLock();
        }
        
        ~SimpleMultThreadVector()
        {
            delete mlock;
        }

        uint32_t GetSize()
        {
            LockGuard(*mlock);
            
            return Data.size();
        }

        void Emplace_Back(ElementType& Element)
        {
            LockGuard(*mlock);
            
            Data.emplace_back(std::forward<ElementType>(Element));
        }

        void Emplace_Back(ElementType&& Element)
        {
            LockGuard(*mlock);
            
            Data.emplace_back(std::forward<ElementType>(Element));
        }

        void Pop_Back()
        {
            LockGuard(*mlock);
            
            Data.pop_back();
        }

        ElementType& Back()
        {
            LockGuard(*mlock);
            
            return Data.back();
        }

        ElementType& operator[](uint32_t Index)
        {
            LockGuard(*mlock);
            
            return Data[Index];
        }
    };
}

