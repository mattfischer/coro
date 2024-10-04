#include "Executor.hpp"

#include <thread>

Executor *Executor::sCurrent = nullptr;

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
    sCurrent = this;
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
    }
    sCurrent = nullptr;
}

Executor::YieldAwaitable Executor::yield()
{
    return YieldAwaitable(*sCurrent);
}
