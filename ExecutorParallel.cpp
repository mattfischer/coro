#include "ExecutorParallel.hpp"
#include "Task.hpp"

#include <functional>

void ExecutorParallel::enqueueTask(Task *task)
{
    std::lock_guard lock(mMutex);

    mReadyQueue.push(task);

    mConditionVariable.notify_one();
}

void ExecutorParallel::enqueueTaskLater(Task *task, std::chrono::steady_clock::time_point wakeup)
{
    std::lock_guard lock(mMutex);

    mLaterQueue.emplace(task, wakeup);

    mConditionVariable.notify_one();
}

void ExecutorParallel::start(unsigned int numThreads)
{
    mRunThreads = true;
    for(int i=0; i<numThreads; i++) {
        mThreads.emplace_back([&]() { runThread(); } );
    }
}

void ExecutorParallel::stop()
{
    {
        std::unique_lock lock(mMutex);
        mRunThreads = false;
        mConditionVariable.notify_all();
    }

    for(auto &thread : mThreads) {
        thread.join();
    }
}

void ExecutorParallel::runThread()
{
    while(true) {
        Task *task = nullptr;
        {
            std::unique_lock lock(mMutex);
            if(!mRunThreads) {
                break;
            }

            if(mReadyQueue.size() > 0) {
                task = mReadyQueue.front();
                mReadyQueue.pop();
            } else if(mLaterQueue.size() > 0) {
                if(mLaterQueue.top().wakeup <= std::chrono::steady_clock::now()) {
                    mReadyQueue.push(mLaterQueue.top().task);
                    mLaterQueue.pop();    
                } else {
                    mConditionVariable.wait_until(lock, mLaterQueue.top().wakeup);
                }                
            } else {
                mConditionVariable.wait(lock);
            }
        }

        if(task) {
            task->run();
        }
    }
}