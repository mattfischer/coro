#include "Task.hpp"

Task *Task::sCurrentTask = nullptr;

Task::Task(std::coroutine_handle<> handle, Executor &executor)
: mExecutor(executor)
{    
    mStartHandle = mResumeHandle = handle;
}

Task::~Task()
{
    mStartHandle.destroy();
}

Executor &Task::executor()
{
    return mExecutor;
}

void Task::run()
{
    sCurrentTask = this;

    mResumeHandle.resume();

    sCurrentTask = nullptr;

    if(mStartHandle.done()) {
        delete this;
    }
}

void Task::suspend(std::coroutine_handle<> resumeHandle)
{
    mResumeHandle = resumeHandle;
}

void Task::enqueueResume()
{
    mExecutor.enqueueTask(this);
}

Task *Task::current()
{
    return sCurrentTask;
}

Task::YieldAwaitable Task::yield()
{
    return YieldAwaitable(sCurrentTask);
}
