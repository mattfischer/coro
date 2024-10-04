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
        struct AsyncRunner;
        template<typename Awaitable> AsyncRunner runAwaitable(Awaitable awaitable);

        std::queue<std::coroutine_handle<>> mReadyQueue;

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

struct Executor::AsyncRunner {
    struct promise_type {
        struct FinalAwaiter {
            bool await_ready() noexcept { return false; }
            void await_suspend(std::coroutine_handle<> handle) noexcept { handle.destroy(); }
            void await_resume() noexcept {}
        };

        AsyncRunner get_return_object() { return { std::coroutine_handle<promise_type>::from_promise(*this) }; }
        std::suspend_always initial_suspend() noexcept { return {}; }
        FinalAwaiter final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };

    std::coroutine_handle<promise_type> handle;
};

template<typename Awaitable> void Executor::runAsync(Awaitable&& awaitable)
{ 
    AsyncRunner helper = runAwaitable<Awaitable>(std::forward<Awaitable>(awaitable));

    enqueue(helper.handle);
}

template<typename Awaitable> Executor::AsyncRunner Executor::runAwaitable(Awaitable awaitable) {
    co_await awaitable;
    co_return;
}

template<typename Rep, typename Period> Executor::SleepAwaitable Executor::sleep_for(const std::chrono::duration<Rep, Period> &duration)
{
    std::chrono::steady_clock::time_point wakeup = std::chrono::steady_clock::now() + duration;
    return { *this, wakeup };
}

#endif