#ifndef AASYNC_HPP
#define ASYNC_HPP

#include <coroutine>

template<typename ReturnType> class Async {
public:
    struct Promise;
    struct Awaiter;
    struct FinalAwaiter;

    using promise_type = Promise;
    using CoroutineHandle = std::coroutine_handle<Promise>;

    Awaiter operator co_await() { return { *this }; }

    Async(CoroutineHandle handle)
    : mHandle(handle)
    {
    }

    Async(Async &&other)
    {
        mHandle = other.mHandle;
        other.mHandle = CoroutineHandle();
    }

    Async &operator=(Async &&other)
    {
        mHandle = other.mHandle;
        other.mHandle = CoroutineHandle();
        return *this;
    }

    ~Async()
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

template<typename ReturnType> struct Async<ReturnType>::Promise {
    Async<ReturnType> get_return_object() { return Async<ReturnType>(CoroutineHandle::from_promise(*this)); }
    std::suspend_always initial_suspend() noexcept { return {}; }
    FinalAwaiter final_suspend() noexcept { return { awaiter }; }
    void return_value(ReturnType value) { returnValue = value; }
    void unhandled_exception() {}

    std::coroutine_handle<> awaiter = std::noop_coroutine();
    ReturnType returnValue;
};

template<> struct Async<void>::Promise {
    Async<void> get_return_object() { return Async<void>(CoroutineHandle::from_promise(*this)); }
    std::suspend_always initial_suspend() noexcept { return {}; }
    Async<void>::FinalAwaiter final_suspend() noexcept { return { awaiter }; }
    void return_void() {}
    void unhandled_exception() {}

    std::coroutine_handle<> awaiter = std::noop_coroutine();
};

template<typename ReturnType> struct Async<ReturnType>::FinalAwaiter {
    bool await_ready() noexcept { return false; }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<>) noexcept { return resumeHandle; }
    void await_resume() noexcept {}

    std::coroutine_handle<> resumeHandle;
};

template<typename ReturnType> struct Async<ReturnType>::Awaiter {
    bool await_ready() { return false; }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle)
    {
        async.mHandle.promise().awaiter = handle; 
        return async.mHandle;   
    }
    ReturnType await_resume() { return async.mHandle.promise().returnValue; }

    Async<ReturnType> &async;
};

template<> struct Async<void>::Awaiter {
    bool await_ready() { return false; }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle)
    {
        async.mHandle.promise().awaiter = handle; 
        return async.mHandle;   
    }
    void await_resume() {}

    Async<void> &async;
};

#endif