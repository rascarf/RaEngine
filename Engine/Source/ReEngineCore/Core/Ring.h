#pragma once
#include "Application.h"

class Ring
{
public:
    void Create(UINT32 TotalSize)
    {
        m_Head = 0;
        m_AllocatedSize = 0;
        m_TotalSize = TotalSize;
    }

    uint32_t GetSize() { return m_AllocatedSize; }
    uint32_t GetHead() { return m_Head; }
    uint32_t GetTail() { return (m_Head + m_AllocatedSize) % m_TotalSize; }


    UINT32 PaddingToAviodCrossOver(UINT32 size)
    {
        int Tail = GetTail();
        if((Tail + size) > m_TotalSize)
        {
            return (m_TotalSize - Tail);
        }
        else
        {
            return 0;
        }
    }

    //@param pout:输出当前分配后尾部在哪里
    bool Alloc(uint32_t size,uint32_t *pOut)
    {
        if(m_AllocatedSize + size <= m_TotalSize)
        {
            if(pOut)
            {
                *pOut = GetTail();
            }

            m_AllocatedSize += size;
            return true;
        }

        RE_CORE_ERROR("{===========m_AllocatedSize + size <= m_TotalSize=======}");
        return false;;
    }

    bool Free(uint32_t size)
    {
        if (m_AllocatedSize >= size)
        {
            m_Head = (m_Head + size) % m_TotalSize;
            m_AllocatedSize -= size;
            return true;
        }
        return false;
    }
    
    
private:
    UINT32 m_Head; //根节点，相当于起点
    UINT32 m_AllocatedSize;
    UINT32 m_TotalSize;
};

class RingWithTab
{
public:
    void OnCreate(uint32_t NumberOfBackBuffers,uint32_t memTotalSize)
    {
        m_BackBufferIndex = 0;
        m_NumberOfBackBuffers = NumberOfBackBuffers;

        m_memAllocatedInFrame = 0;
        for(int i = 0 ; i < 4 ; i++)
        {
            m_AllocatedMemPerBackBuffer[i] = 0;
        }

        m_mem.Create(memTotalSize);
    }

    void OnDestroy()
    {
        m_mem.Free(m_mem.GetSize());
    }

    bool Alloc(uint32_t size, uint32_t *pOut)
    {
        uint32_t Padding = m_mem.PaddingToAviodCrossOver(size);
        if(Padding > 0)
        {
            m_memAllocatedInFrame += Padding;

            if(m_mem.Alloc(Padding,NULL) == false)
            {
                return false;
            }
        }

        if(m_mem.Alloc(size,pOut) == true)
        {
            m_memAllocatedInFrame += size;
            return true;
        }

        return false;
    }

    void OnBeginFrame()
    {
        m_AllocatedMemPerBackBuffer[m_BackBufferIndex] = m_memAllocatedInFrame;
        m_memAllocatedInFrame = 0;
        m_BackBufferIndex = (m_BackBufferIndex + 1) % m_NumberOfBackBuffers;

        // free all the entries for the oldest buffer in one go
        uint32_t memToFree = m_AllocatedMemPerBackBuffer[m_BackBufferIndex];
        m_mem.Free(memToFree);
    }

private:
    Ring m_mem;

    //当前的Index
    uint32_t m_BackBufferIndex;

    //一共有多少FrameBuffer
    uint32_t m_NumberOfBackBuffers;

    //当前帧分配的一共是多少
    uint32_t m_memAllocatedInFrame;

    //保存了前三帧的分配情况
    uint32_t m_AllocatedMemPerBackBuffer[4];
};
