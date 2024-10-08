#include "ExecutorSerial.hpp"

#include "Task.hpp"

#include <thread>

void ExecutorSerial::enqueueTask(Task *task)
{
    mReadyQueue.push(task);
}

void ExecutorSerial::enqueueTaskLater(Task *task, std::chrono::steady_clock::time_point wakeup)
{
    mLaterQueue.emplace(task, wakeup);
}

void ExecutorSerial::exec()
{
    while(true) {
        if(mReadyQueue.size() > 0) {
            Task *task = mReadyQueue.front();
            mReadyQueue.pop();

            task->run();   
        } else if(mLaterQueue.size() > 0) {
            std::this_thread::sleep_until(mLaterQueue.top().wakeup);
            mReadyQueue.push(mLaterQueue.top().task);
            mLaterQueue.pop();
        } else {
            break;
        }
    }
}
