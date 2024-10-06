#ifndef FUTURE_HPP
#define FUTURE_HPP

#include "Task.hpp"

#include <vector>

template<typename ReturnType>
class Future {
public:
    Future()
    {
        mReady = false;
    }

    bool await_ready() { return mReady; }
    void await_suspend(std::coroutine_handle<> handle) 
    { 
        Task *task = Task::current();
        task->suspend(handle);
        mAwaiters.push_back(task);
    }

    ReturnType await_resume() { return mReturnValue; }

    void complete(ReturnType returnValue)
    {
        mReady = true;
        mReturnValue = returnValue;
        for(Task *task : mAwaiters)
        {
            task->enqueueResume();
        }
    }

private:
    bool mReady;
    ReturnType mReturnValue;
    std::vector<Task*> mAwaiters;
};

template<>
class Future<void> {
public:
    Future()
    {
        mReady = false;
    }

    bool await_ready() { return mReady; }
    void await_suspend(std::coroutine_handle<> handle) {
        Task *task = Task::current();
        task->suspend(handle);
        mAwaiters.push_back(task);
    }
    void await_resume() {}

    void complete()
    {
        mReady = true;
        for(Task *task : mAwaiters)
        {
            task->enqueueResume();
        }
    }

private:
    bool mReady;
    std::vector<Task*> mAwaiters;
};

#endif