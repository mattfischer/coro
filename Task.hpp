#ifndef TASK_HPP
#define TASK_HPP

#include "Async.hpp"
#include "Executor.hpp"

#include <coroutine>
#include <chrono>

class Task {
public:
    void run();

    void enqueueResume();

    template<typename ReturnType> static void start(Async<ReturnType> async, Executor &executor) {
        Task *task = new Task(async.releaseHandle(), executor);
        executor.enqueueTask(task);
    }

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

    static thread_local Task *sCurrent;
};

struct Task::YieldAwaitable {
    bool await_ready() { return false; }
    void await_resume() {}

    void await_suspend(std::coroutine_handle<> handle) {
        Task *task = Task::suspend(handle);
        task->mExecutor.enqueueTask(task);
    }
};

struct Task::SleepAwaitable {
    bool await_ready() { return false; }
    void await_resume() {}

    void await_suspend(std::coroutine_handle<> handle) {
        Task *task = Task::suspend(handle);
        task->mExecutor.enqueueTaskLater(task, wakeup);
    }
    
    std::chrono::steady_clock::time_point wakeup;
};

#endif