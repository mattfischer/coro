#ifndef ASYNC_QUEUE_HPP
#define ASYNC_QUEUE_HPP

#include "Task.hpp"

#include <queue>

template<typename ElementType>
class AsyncQueue {
public:
    struct Awaitable;

    Awaitable operator co_await() { return { *this }; }

    void enqueue(ElementType element)
    {
        mQueue.push(element);

        if(mAwaiters.size() > 0) {
            Task *task = mAwaiters.front();
            mAwaiters.pop();
            task->enqueueResume();
        }
    }

private:
    std::queue<ElementType> mQueue;
    std::queue<Task*> mAwaiters;
};

template<typename ElementType>
struct AsyncQueue<ElementType>::Awaitable {
    bool await_ready()
    {
        return queue.mQueue.size() > 0;
    }

    void await_suspend(std::coroutine_handle<> resumeHandle)
    {
        Task *task = Task::current();
        task->suspend(resumeHandle);
        queue.mAwaiters.push(task);
    }

    ElementType await_resume()
    {
        ElementType result = queue.mQueue.front();
        queue.mQueue.pop();

        if(queue.mAwaiters.size() > 0 && queue.mQueue.size() > 0) {
            Task *task = queue.mAwaiters.front();
            queue.mAwaiters.pop();
            task->enqueueResume();
        }

        return result;
    }

AsyncQueue &queue;
};

#endif