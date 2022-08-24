#pragma once
#include <functional>
#include <queue>

namespace ReEngine
{
    class RenderCommandQueue
    {
    public:
        template<typename FuncT>
        void Submit(FuncT&& func)
        {
            m_CommandQueue.emplace(func);
        }

        void Execute()
        {
            while (!m_CommandQueue.empty())
            {
                m_CommandQueue.front()();
                m_CommandQueue.pop();
            }
        }


    private:
        std::queue<std::function<void()>> m_CommandQueue;
    };
}
