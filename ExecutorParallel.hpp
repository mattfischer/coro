#ifndef EXECUTOR_PARALLEL_HPP
#define EXECUTOR_PARALLEL_HPP

#include "Executor.hpp"

#include <queue>
#include <mutex>
#include <thread>
#include <vector>

class ExecutorParallel : public Executor {
public:
    void enqueueTask(Task *task) override;
    void enqueueTaskLater(Task *task, std::chrono::steady_clock::time_point wakeup) override;

    void start(unsigned int numThreads);
    void stop();

private:
    void runThread();

    std::queue<Task*> mReadyQueue;

    struct LaterEntry {
        Task *task;
        std::chrono::steady_clock::time_point wakeup;
        
        bool operator>(const LaterEntry &other) const {
            return wakeup > other.wakeup;
        }
    };
    std::priority_queue<LaterEntry, std::vector<LaterEntry>, std::greater<LaterEntry>> mLaterQueue;
    std::vector<std::thread> mThreads;
    bool mRunThreads;
    std::mutex mMutex;
    std::condition_variable mConditionVariable;
};

#endif