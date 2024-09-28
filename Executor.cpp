#include "Executor.hpp"

#include <thread>

void Executor::enqueue(std::coroutine_handle<> handle)
{
    mReadyQueue.push(handle);
}

void Executor::enqueueLater(std::coroutine_handle<> handle, std::chrono::steady_clock::time_point wakeup)
{
    mLaterQueue.emplace(handle, wakeup);
}

void Executor::exec()
{
    while(true) {
        if(mReadyQueue.size() > 0) {
            mReadyQueue.front().resume();
            mReadyQueue.pop();
        } else if(mLaterQueue.size() > 0) {
            std::this_thread::sleep_until(mLaterQueue.top().wakeup);
            mLaterQueue.top().handle.resume();
            mLaterQueue.pop();
        } else {
            break;
        }

        if(mCleanupHandles.size() > 0) {
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
