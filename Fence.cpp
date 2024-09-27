#include "Fence.hpp"

Fence::Fence(Executor &executor)
: mExecutor(executor)
{
    mReady = false;
}

bool Fence::await_ready()
{
    return mReady;
}

void Fence::await_suspend(std::coroutine_handle<> handle)
{
    mHandle = handle;
}

void Fence::await_resume()
{
}

void Fence::signal()
{
    mReady = true;
    if(mHandle) {
        mExecutor.queue(mHandle);
    }
}
