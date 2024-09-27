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

        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(void *) { return {}; }
        void return_void() {}
        void unhandled_exception() {}
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

    void run()
    {
        mHandle.resume();
    }

private:
    CoroutineHandle mHandle;
};

#endif