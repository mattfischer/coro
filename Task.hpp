#ifndef TASK_HPP
#define TASK_HPP

#include <coroutine>

template<typename ReturnType> class Task {
public:
    struct Promise;
    struct Awaiter;
    struct FinalAwaiter;

    using promise_type = Promise;
    using CoroutineHandle = std::coroutine_handle<Promise>;

    Awaiter operator co_await() { return {*this}; }

    Task(CoroutineHandle handle)
    : mHandle(handle)
    {
    }

    ~Task()
    {
        if(mHandle) {
            mHandle.destroy();
        }
    }

    const CoroutineHandle &handle() const
    {
        return mHandle;
    }

private:
    CoroutineHandle mHandle;
};

template<typename ReturnType> struct Task<ReturnType>::Promise {
    Task<ReturnType> get_return_object()
    {
        return Task<ReturnType>(CoroutineHandle::from_promise(*this));
    }

    std::suspend_always initial_suspend() noexcept
    {
        return {};
    }

    FinalAwaiter final_suspend() noexcept
    {
        return {awaiter};
    }

    void return_value(ReturnType value) 
    {
        returnValue = value;
    }

    void unhandled_exception()
    {
    }

    std::coroutine_handle<> awaiter = std::noop_coroutine();
    ReturnType returnValue;
};

template<> struct Task<void>::Promise {
    Task<void> get_return_object()
    {
        return Task<void>(CoroutineHandle::from_promise(*this));
    }

    std::suspend_always initial_suspend() noexcept
    {
        return {};
    }

    Task<void>::FinalAwaiter final_suspend() noexcept
    {
        return {awaiter};
    }

    void return_void() 
    {
    }

    void unhandled_exception()
    {
    }

    std::coroutine_handle<> awaiter = std::noop_coroutine();
};

template<typename ReturnType> struct Task<ReturnType>::FinalAwaiter {
    bool await_ready() noexcept
    {
        return false;
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept
    {
        return resumeHandle;
    }

    void await_resume() noexcept
    {
    }

    std::coroutine_handle<> resumeHandle;
};

template<typename ReturnType> struct Task<ReturnType>::Awaiter {
    bool await_ready() 
    {
        return false;
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle)
    {
        task.mHandle.promise().awaiter = handle; 
        return task.mHandle;   
    }

    ReturnType await_resume()
    {
        return task.mHandle.promise().returnValue;
    }

    Task<ReturnType> &task;
};

template<> struct Task<void>::Awaiter {
    bool await_ready() 
    {
        return false;
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle)
    {
        task.mHandle.promise().awaiter = handle; 
        return task.mHandle;   
    }

    void await_resume()
    {
    }

    Task<void> &task;
};

#endif