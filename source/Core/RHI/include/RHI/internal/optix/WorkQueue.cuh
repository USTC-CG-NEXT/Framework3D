#pragma once

#ifndef __CUDACC__
#include <cuda_runtime_api.h>
#endif

#include <cuda/std/atomic>

template<typename T>
struct WorkQueue {
    T* ptr = nullptr;

    WorkQueue(T* ptr) : ptr(ptr)
    {
        size.store(0);
    }

    WorkQueue(const WorkQueue& queue)
    {
        ptr = queue.ptr;
        size.store(queue.size.load());
    }

    WorkQueue& operator=(const WorkQueue& queue)
    {
        ptr = queue.ptr;
        size.store(queue.size.load());
        return *this;
    }

    void SetSize(int size_)
    {
        size.store(size_);
    }
    cuda::std::atomic<int> size{ 0 };

    __host__ __device__ int Size() const
    {
        return size.load(cuda::std::memory_order_relaxed);
    }

    void Reset()
    {
        size.store(0, cuda::std::memory_order_relaxed);
    }

    __host__ __device__ int Push(T w)
    {
        int index = AllocateEntry();
        ptr[index] = w;
        return index;
    }

   protected:
    // WorkQueue Protected Methods
    __host__ __device__ int AllocateEntry()
    {
        return size.fetch_add(1, cuda::std::memory_order_relaxed);
    }
};
