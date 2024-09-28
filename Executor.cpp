#include "Executor.hpp"

void Executor::enqueue(std::coroutine_handle<> handle)
{
    mQueue.push(handle);
}

void Executor::exec()
{
    while(!mQueue.empty()) {
        mQueue.front().resume();
        mQueue.pop();

        if(!mCleanupHandles.empty()) {
            for(auto &handle : mCleanupHandles) {
                handle.destroy();
            }
            mCleanupHandles.clear();
        }
    }
}

Executor::YieldAwaitable Executor::yield()
{
    return YieldAwaitable(*this);
}

void Executor::cleanupHandle(std::coroutine_handle<> handle)
{
    mCleanupHandles.push_back(handle);
}
