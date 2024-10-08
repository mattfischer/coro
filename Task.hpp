#ifndef TASK_HPP
#define TASK_HPP

#include "Async.hpp"
#include "Executor.hpp"

#include <coroutine>
#include <chrono>

class Task {
public:
    Executor &executor();
    void run();

    void enqueueResume();

    template<typename ReturnType> static void start(Async<ReturnType>&& async, Executor &executor) {
        AsyncRunner runner = runAsync<ReturnType>(std::forward<Async<ReturnType>>(async));
        Task *task = new Task(runner.handle, executor);
        executor.enqueueTask(task);
    }

    static Task *current();
    static Task *suspend(std::coroutine_handle<> resumeHandle);

    struct YieldAwaitable;
    static YieldAwaitable yield();

    struct SleepAwaitable;
    template<typename Rep, typename Period> static SleepAwaitable sleep_for(const std::chrono::duration<Rep, Period> &duration)
    {
        std::chrono::steady_clock::time_point wakeup = std::chrono::steady_clock::now() + duration;
        return { wakeup };
    }

private:
    Task(std::coroutine_handle<> handle, Executor &executor);
    ~Task();

    Executor &mExecutor;
    std::coroutine_handle<> mStartHandle;
    std::coroutine_handle<> mResumeHandle;

    static Task *sCurrent;

    struct AsyncRunner;
    template<typename ReturnType> static AsyncRunner runAsync(Async<ReturnType> async)
    {
        co_await async;
        co_return;
    }
};

struct Task::YieldAwaitable {
    bool await_ready() { return false; }
    void await_resume() {}

    void await_suspend(std::coroutine_handle<> handle) {
        Task *task = Task::suspend(handle);
        task->executor().enqueueTask(task);
    }
};

struct Task::SleepAwaitable {
    bool await_ready() { return false; }
    void await_resume() {}

    void await_suspend(std::coroutine_handle<> handle) {
        Task *task = Task::suspend(handle);
        task->executor().enqueueTaskLater(task, wakeup);
    }
    
    std::chrono::steady_clock::time_point wakeup;
};

struct Task::AsyncRunner {
    struct promise_type {
        AsyncRunner get_return_object() { return { std::coroutine_handle<promise_type>::from_promise(*this) }; }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };

    std::coroutine_handle<> handle;
};

#endif