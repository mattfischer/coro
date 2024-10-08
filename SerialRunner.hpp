#ifndef SERIAL_RUNNER_HPP
#define SERIAL_RUNNER_HPP

#include "Async.hpp"
#include "Task.hpp"
#include "Executor.hpp"
#include "AsyncQueue.hpp"

class SerialRunner {
public:
    SerialRunner(Executor &executor) {
        Task::start(runLoop(), executor);
    }

    struct RunItem;
    template<typename ReturnType> struct Awaitable;

    template<typename ReturnType> Awaitable<ReturnType> runAsync(Async<ReturnType>&& async) {
        Awaitable<ReturnType> awaitable(*this, std::forward<Async<ReturnType>>(async));

        return awaitable;
    }

private:
    AsyncQueue<RunItem*> mQueue;
    Async<void> runLoop();
};

struct SerialRunner::RunItem {
    virtual Async<void> run() = 0;
};

template<typename ReturnType> class SerialRunner::Awaitable : public RunItem
{
public:
    Awaitable(SerialRunner &runner, Async<ReturnType>&& async)
    : mRunner(runner), mAsync(std::forward<Async<ReturnType>>(async))
    {
    }

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> resumeHandle) {
        mAwaiter = Task::suspend(resumeHandle);
        mRunner.mQueue.enqueue(this);
    }
    ReturnType await_resume() { return mReturnValue; }

    Async<void> run() override {
        mReturnValue = co_await mAsync;
        mAwaiter->enqueueResume();
    }

private:
    Task *mAwaiter;
    SerialRunner &mRunner;
    Async<ReturnType> mAsync;
    ReturnType mReturnValue;
};

template<> class SerialRunner::Awaitable<void> : public RunItem
{
public:
    Awaitable(SerialRunner &runner, Async<void>&& async)
    : mRunner(runner), mAsync(std::forward<Async<void>>(async))
    {
    }

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> resumeHandle) {
        mAwaiter = Task::suspend(resumeHandle);
        mRunner.mQueue.enqueue(this);
    }
    void await_resume() {}

    Async<void> run() override {
        co_await mAsync;
        mAwaiter->enqueueResume();
    }

private:
    Task *mAwaiter;
    SerialRunner &mRunner;
    Async<void> mAsync;
};

Async<void> SerialRunner::runLoop() {
    while(true) {
        RunItem *item = co_await mQueue;
        co_await item->run();
    }
    co_return;
}

#endif