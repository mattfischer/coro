#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include <queue>
#include <coroutine>
#include <utility>

class Executor {
    public:
        void enqueue(std::coroutine_handle<> handle);
        template<typename Awaitable> void runAsync(Awaitable&& awaitable);

        void exec();

        struct YieldAwaitable;
        YieldAwaitable yield();

    private:
        struct RunHelper;
        template<typename Awaitable> RunHelper runAwaitable(Awaitable awaitable);

        void cleanupHandle(std::coroutine_handle<> handle);

        std::queue<std::coroutine_handle<>> mQueue;
        std::vector<std::coroutine_handle<>> mCleanupHandles;
};

struct Executor::YieldAwaitable {
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> handle) { executor.enqueue(handle); }
    void await_resume() {}

    Executor &executor;
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

#endif