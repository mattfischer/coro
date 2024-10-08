#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "Task.hpp"

#include <mutex>
#include <queue>

template<typename ElementType>
class Queue {
public:
    struct Awaitable;

    Awaitable operator co_await() { return { *this }; }

    void enqueue(ElementType element)
    {
        std::lock_guard lock(mMutex);

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
    std::mutex mMutex;
};

template<typename ElementType>
struct Queue<ElementType>::Awaitable {
    bool await_ready()
    {
        std::lock_guard lock(queue.mMutex);

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
        std::lock_guard lock(queue.mMutex);

        task = Task::suspend(resumeHandle);
        queue.mAwaitableQueue.push(this);
    }

    ElementType await_resume()
    {
        return element;
    }

    Queue &queue;
    Task *task;
    ElementType element;
};

#endif