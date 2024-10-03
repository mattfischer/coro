#ifndef FUTURE_HPP
#define FUTURE_HPP

#include "Executor.hpp"

#include <vector>

template<typename ReturnType>
class Future {
public:
    Future(Executor &executor)
    : mExecutor(executor)
    {
        mReady = false;
    }


    bool await_ready() { return mReady; }
    void await_suspend(std::coroutine_handle<> handle) { mAwaiters.push_back(handle); }
    ReturnType await_resume() { return mReturnValue; }

    void complete(ReturnType returnValue)
    {
        mReady = true;
        mReturnValue = returnValue;
        for(auto &awaiter : mAwaiters)
        {
            mExecutor.enqueue(awaiter);
        }
    }

private:
    bool mReady;
    Executor &mExecutor;
    ReturnType mReturnValue;
    std::vector<std::coroutine_handle<>> mAwaiters;
};

template<>
class Future<void> {
public:
    Future(Executor &executor)
    : mExecutor(executor)
    {
        mReady = false;
    }


    bool await_ready() { return mReady; }
    void await_suspend(std::coroutine_handle<> handle) { mAwaiters.push_back(handle); }
    void await_resume() {}

    void complete()
    {
        mReady = true;
        for(auto &awaiter : mAwaiters)
        {
            mExecutor.enqueue(awaiter);
        }
    }

private:
    bool mReady;
    Executor &mExecutor;
    std::vector<std::coroutine_handle<>> mAwaiters;
};

#endif