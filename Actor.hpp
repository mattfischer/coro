#ifndef ACTOR_HPP
#define ACTOR_HPP

#include "Async.hpp"
#include "Task.hpp"
#include "Executor.hpp"
#include "Queue.hpp"

class Actor {
public:
    Actor(Executor &executor) {
        Task::start(runLoop(), executor);
    }

    struct RunItem;
    template<typename ReturnType> struct Awaitable;

    template<typename ReturnType> Awaitable<ReturnType> run(Async<ReturnType> async) {
        Awaitable<ReturnType> awaitable(*this, std::move(async));

        return awaitable;
    }

private:
    Queue<RunItem*> mQueue;
    Async<void> runLoop();
};

struct Actor::RunItem {
    virtual Async<void> run() = 0;
};

template<typename ReturnType> class Actor::Awaitable : public RunItem
{
public:
    Awaitable(Actor &actor, Async<ReturnType> async)
    : mActor(actor), mAsync(std::move(async))
    {
    }

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> resumeHandle) {
        mAwaiter = Task::suspend(resumeHandle);
        mActor.mQueue.enqueue(this);
    }
    ReturnType await_resume() { return mReturnValue; }

    Async<void> run() override {
        mReturnValue = co_await mAsync;
        mAwaiter->enqueueResume();
    }

private:
    Task *mAwaiter;
    Actor &mActor;
    Async<ReturnType> mAsync;
    ReturnType mReturnValue;
};

template<> class Actor::Awaitable<void> : public RunItem
{
public:
    Awaitable(Actor &actor, Async<void>&& async)
    : mActor(actor), mAsync(std::forward<Async<void>>(async))
    {
    }

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> resumeHandle) {
        mAwaiter = Task::suspend(resumeHandle);
        mActor.mQueue.enqueue(this);
    }
    void await_resume() {}

    Async<void> run() override {
        co_await mAsync;
        mAwaiter->enqueueResume();
    }

private:
    Task *mAwaiter;
    Actor &mActor;
    Async<void> mAsync;
};

Async<void> Actor::runLoop() {
    while(true) {
        RunItem *item = co_await mQueue;
        co_await item->run();
    }
    co_return;
}

#endif