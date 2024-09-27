#ifndef FENCE_HPP
#define FENCE_HPP

#include "Executor.hpp"

class Fence {
public:
    Fence(Executor &executor);

    bool await_ready();
    void await_suspend(std::coroutine_handle<> handle);
    void await_resume();

    void signal();

private:
    bool mReady;
    Executor &mExecutor;
    std::coroutine_handle<> mHandle;
};

#endif