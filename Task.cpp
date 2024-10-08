#include "Task.hpp"

thread_local Task *Task::sCurrent = nullptr;

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
    sCurrent = this;

    mResumeHandle.resume();

    sCurrent = nullptr;

    if(mStartHandle.done()) {
        delete this;
    }
}

Task *Task::suspend(std::coroutine_handle<> resumeHandle)
{
    Task *task = sCurrent;
    task->mResumeHandle = resumeHandle;

    return task;
}

void Task::enqueueResume()
{
    mExecutor.enqueueTask(this);
}

Task::YieldAwaitable Task::yield()
{
    return YieldAwaitable();
}
