#ifndef TASK_HPP
#define TASK_HPP

#include <coroutine>

class Task {
public:
    struct promise_type;
    using CoroutineHandle = std::coroutine_handle<promise_type>;

    struct promise_type {
        Task get_return_object() {
            return Task(CoroutineHandle::from_promise(*this));
        }

        std::suspend_always initial_suspend() noexcept { runnable = true; return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(void *) { return {}; }
        void return_void() {}
        void unhandled_exception() {}

        bool runnable;
    };

    struct Fence {
        Fence() { mReady = false; }

        bool await_ready() {
            return mReady;
        }

        void await_suspend(CoroutineHandle handle) {
            mHandle = handle;
            mHandle.promise().runnable = false;
        }

        void await_resume() {}

        void signal()
        {
            mReady = true;
            if(mHandle) {
                mHandle.promise().runnable = true;
            }
        }

        bool mReady;
        CoroutineHandle mHandle;
    };

    Task(CoroutineHandle handle)
    : mHandle(handle)
    {
    }

    Task(Task &&other)
    {
        mHandle = other.mHandle;
        other.mHandle = CoroutineHandle();
    }

    ~Task()
    {
        if(mHandle) {
            mHandle.destroy();
        }
    }

    Task &operator=(Task &&other) {
        mHandle = other.mHandle;
        other.mHandle = CoroutineHandle();
        return *this;
    }

    bool done() {
        return mHandle ? mHandle.done() : true;
    }

    bool runnable() {
        return mHandle.promise().runnable;
    }

    void run()
    {
        mHandle.resume();
    }

private:
    CoroutineHandle mHandle;
};

#endif