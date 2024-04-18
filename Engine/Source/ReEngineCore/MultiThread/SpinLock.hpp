#pragma once
#include <atomic>

namespace ReEngine
{
    // 非公平，但是够用了
    class SpinLock
    {
    public:
        SpinLock(){Flag.store(false);};

        SpinLock(const SpinLock &other) = delete;
        SpinLock(SpinLock &&other) = delete;
        SpinLock &operator= (const SpinLock &other) = delete;
        SpinLock &operator= (SpinLock &&other) = delete;

        void Lock()
        {
            bool Expected = false;
            // Flag == Expected,Flag = true,return true
            // Flag != Expected,Expected = Flag,return false
            while(!Flag.compare_exchange_weak(Expected,true))
            {
                //compare_exchange_weak方法执行CAS操作失败，会将当前原子变量的值存储在expected中
                Expected = false;

                // 提示当前正在自旋
                _mm_pause();
            }
        }
        
        void UnLock()
        {
            Flag.store(false);
        }

    private:
        std::atomic<bool> Flag;
    };

    //  构造函数加锁，析构函数解锁
    class LockGuard
    {
        SpinLock*  _m;
    public:
        explicit LockGuard(SpinLock& InLock):
            _m(&InLock)
        {
            _m->Lock();
        }

        LockGuard(const LockGuard&) = delete;
        LockGuard(LockGuard&&) = delete;
        LockGuard& operator=(const LockGuard&) = delete;
        LockGuard& operator=(LockGuard&&) = delete;

        void UnLock()
        {
            if(_m)
            {
                _m->UnLock();
                _m = nullptr;
            }
        }

        LockGuard& operator=(SpinLock& InLock)
        {
            UnLock();
            _m = &InLock;
            _m->Lock();
            return *this;
        }

        ~LockGuard()
        {
            UnLock();
        }
    };
    
};
