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
        if(mAwaitableQueue.size() > 0) {
            Awaitable *awaitable = mAwaitableQueue.front();
            mAwaitableQueue.pop();

            awaitable->element = element;
            awaitable->task->enqueueResume();
        } else {
            mElementQueue.push(element);
        }
    }

private:
    std::queue<ElementType> mElementQueue;
    std::queue<Awaitable*> mAwaitableQueue;
};

template<typename ElementType>
struct AsyncQueue<ElementType>::Awaitable {
    bool await_ready()
    {
        if(queue.mElementQueue.size() > 0) {
            element = queue.mElementQueue.front();
            queue.mElementQueue.pop();
            return true;
        } else {
            return false;
        }
    }

    void await_suspend(std::coroutine_handle<> resumeHandle)
    {
        task = Task::suspend(resumeHandle);
        queue.mAwaitableQueue.push(this);
    }

    ElementType await_resume()
    {
        return element;
    }

    AsyncQueue &queue;
    Task *task;
    ElementType element;
};

#endif