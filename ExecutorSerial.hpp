#ifndef EXECUTOR_SERIAL_HPP
#define EXECUTOR_SERIAL_HPP

#include "Executor.hpp"

#include <queue>

class ExecutorSerial : public Executor {
    public:
        void enqueueTask(Task *task) override;
        void enqueueTaskLater(Task *task, std::chrono::steady_clock::time_point wakeup) override;

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