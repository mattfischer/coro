#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include <queue>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>

class Task;

class Executor {
public:
    ~Executor();

    void enqueueTask(Task *task);
    void enqueueTaskLater(Task *task, std::chrono::steady_clock::time_point wakeup);

    void start(unsigned int numThreads, bool joinPool);
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