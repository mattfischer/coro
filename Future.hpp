#ifndef FUTURE_HPP
#define FUTURE_HPP

#include "Task.hpp"

#include <vector>
#include <mutex>

template<typename ReturnType>
class Future {
public:
    Future()
    {
        mReady = false;
    }

    struct Awaitable {
        bool await_ready()
        {
            std::lock_guard lock(future.mMutex);

            return future.mReady;
        }

        void await_suspend(std::coroutine_handle<> handle) 
        { 
            std::lock_guard lock(future.mMutex);

            Task *task = Task::suspend(handle);
            future.mAwaiters.push_back(task);
        }

        ReturnType await_resume()
        {
            std::lock_guard lock(future.mMutex);

            return future.mReturnValue;
        }

        Future &future;
    };
    Awaitable operator co_await() { return { *this }; }

    void complete(ReturnType returnValue)
    {
        std::lock_guard lock(mMutex);

        mReady = true;
        mReturnValue = returnValue;
        for(Task *task : mAwaiters)
        {
            task->enqueueResume();
        }
        mAwaiters.clear();
    }

private:
    bool mReady;
    ReturnType mReturnValue;
    std::vector<Task*> mAwaiters;
    std::mutex mMutex;
};

template<>
class Future<void> {
public:
    Future()
    {
        mReady = false;
    }

    struct Awaitable {
        bool await_ready()
        {
            std::lock_guard lock(future.mMutex);

            return future.mReady;
        }

        void await_suspend(std::coroutine_handle<> handle)
        {
            std::lock_guard lock(future.mMutex);

            Task *task = Task::suspend(handle);
            future.mAwaiters.push_back(task);
        }

        void await_resume() {}

        Future<void> &future;
    };
    Awaitable operator co_await() { return { *this }; }

    void complete()
    {
        std::lock_guard lock(mMutex);

        mReady = true;
        for(Task *task : mAwaiters)
        {
            task->enqueueResume();
        }
        mAwaiters.clear();
    }

private:
    bool mReady;
    std::vector<Task*> mAwaiters;
    std::mutex mMutex;
};

#endif