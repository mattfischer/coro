#include "Executor.hpp"

void Executor::queue(std::coroutine_handle<> handle)
{
    mQueue.push(handle);
}

void Executor::run()
{
    while(!mQueue.empty()) {
        mQueue.front().resume();
        mQueue.pop();
    }
}

Executor::YieldAwaitable Executor::yield()
{
    return YieldAwaitable(*this);
}