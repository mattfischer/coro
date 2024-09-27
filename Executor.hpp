#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include <queue>
#include <coroutine>
class Executor {
    public:
        void queue(std::coroutine_handle<> handle);

        void run();

        struct YieldAwaitable {
            bool await_ready() { return false; }
            void await_suspend(std::coroutine_handle<> handle) {
                executor.queue(handle);
            }
            void await_resume() {}

            Executor &executor;
        };

        YieldAwaitable yield();

    private:
        std::queue<std::coroutine_handle<>> mQueue;
};

#endif