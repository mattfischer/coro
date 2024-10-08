#ifndef ACTOR_HPP
#define ACTOR_HPP

#include "Async.hpp"
#include "Task.hpp"
#include "Executor.hpp"
#include "Queue.hpp"

class Actor {
public:
    Actor(Executor &executor);

    template<typename ReturnType> struct Awaitable;
    template<typename ReturnType> Awaitable<ReturnType> run(Async<ReturnType> async)
    {
        Awaitable<ReturnType> awaitable(*this, std::move(async));

        return awaitable;
    }

private:
    struct RunItem
    {
        virtual Async<void> run() = 0;
    };

    Queue<RunItem*> mQueue;

    Async<void> runLoop();
};

template<typename ReturnType> class Actor::Awaitable : public RunItem
{
public:
    Awaitable(Actor &actor, Async<ReturnType> async)
    : mActor(actor), mAsync(std::move(async))
    {
    }

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> resumeHandle)
    {
        mTask = Task::suspend(resumeHandle);
        mActor.mQueue.enqueue(this);
    }
    ReturnType await_resume() { return mReturnValue; }

    Async<void> run() override
    {
        mReturnValue = co_await mAsync;
        mTask->enqueueResume();
    }

private:
    Task *mTask;
    Actor &mActor;
    Async<ReturnType> mAsync;
    ReturnType mReturnValue;
};

template<> class Actor::Awaitable<void> : public RunItem
{
public:
    Awaitable(Actor &actor, Async<void> async)
    : mActor(actor), mAsync(std::move(async))
    {
    }

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> resumeHandle)
    {
        mTask = Task::suspend(resumeHandle);
        mActor.mQueue.enqueue(this);
    }
    void await_resume() {}

    Async<void> run() override
    {
        co_await mAsync;
        mTask->enqueueResume();
    }

private:
    Task *mTask;
    Actor &mActor;
    Async<void> mAsync;
};

#endif