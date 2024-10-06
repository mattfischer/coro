#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include <queue>
#include <coroutine>
#include <utility>
#include <chrono>

class Task;

class Executor {
    public:
        void enqueueTask(Task *task);
        void enqueueTaskLater(Task *task, std::chrono::steady_clock::time_point wakeup);

        void exec();

    private:
        std::queue<Task*> mReadyQueue;

        struct LaterEntry {
            Task *task;
            std::chrono::steady_clock::time_point wakeup;
            
            bool operator>(const LaterEntry &other) const {
                return wakeup > other.wakeup;
            }
        };
        std::priority_queue<LaterEntry, std::vector<LaterEntry>, std::greater<LaterEntry>> mLaterQueue;
};

#endif