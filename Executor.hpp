#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include <queue>
#include <coroutine>
#include <utility>
#include <chrono>

class Executor {
    public:
        void enqueue(std::coroutine_handle<> handle);
        void enqueueLater(std::coroutine_handle<> handle, std::chrono::steady_clock::time_point wakeup);

        template<typename Awaitable> void runAsync(Awaitable&& awaitable);

        void exec();

        struct YieldAwaitable;
        YieldAwaitable yield();

        struct SleepAwaitable;
        template<typename Rep, typename Period> SleepAwaitable sleep_for(const std::chrono::duration<Rep, Period> &duration);

    private:
        struct RunHelper;
        template<typename Awaitable> RunHelper runAwaitable(Awaitable awaitable);

        void cleanupHandle(std::coroutine_handle<> handle);

        std::queue<std::coroutine_handle<>> mReadyQueue;
        std::vector<std::coroutine_handle<>> mCleanupHandles;

        struct LaterEntry {
            std::coroutine_handle<> handle;
            std::chrono::steady_clock::time_point wakeup;
            
            bool operator>(const LaterEntry &other) const {
                return wakeup > other.wakeup;
            }
        };
        std::priority_queue<LaterEntry, std::vector<LaterEntry>, std::greater<LaterEntry>> mLaterQueue;
};

struct Executor::YieldAwaitable {
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> handle) { executor.enqueue(handle); }
    void await_resume() {}

    Executor &executor;
};

struct Executor::SleepAwaitable {
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> handle) { executor.enqueueLater(handle, wakeup); }
    void await_resume() {}

    Executor &executor;
    std::chrono::steady_clock::time_point wakeup;
};

struct Executor::RunHelper {
    struct promise_type {
        struct FinalAwaiter {
            bool await_ready() noexcept { return false; }
            void await_suspend(std::coroutine_handle<> handle) noexcept { executor->cleanupHandle(handle); }
            void await_resume() noexcept {}

            Executor *executor;
        };

        RunHelper get_return_object() { return { std::coroutine_handle<promise_type>::from_promise(*this) }; }
        std::suspend_always initial_suspend() noexcept { return {}; }
        FinalAwaiter final_suspend() noexcept { return { executor }; }
        void return_void() {}
        void unhandled_exception() {}

        Executor *executor;
    };

    std::coroutine_handle<promise_type> handle;
};

template<typename Awaitable> void Executor::runAsync(Awaitable&& awaitable)
{ 
    RunHelper helper = runAwaitable<Awaitable>(std::forward<Awaitable>(awaitable));
    helper.handle.promise().executor = this;

    enqueue(helper.handle);
}

template<typename Awaitable> Executor::RunHelper Executor::runAwaitable(Awaitable awaitable) {
    co_await awaitable;
    co_return;
}

template<typename Rep, typename Period> Executor::SleepAwaitable Executor::sleep_for(const std::chrono::duration<Rep, Period> &duration)
{
    std::chrono::steady_clock::time_point wakeup = std::chrono::steady_clock::now() + duration;
    return { *this, wakeup };
}

#endif