#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include <chrono>

class Task;
class Executor {
public:
    virtual void enqueueTask(Task *task) = 0;
    virtual void enqueueTaskLater(Task *task, std::chrono::steady_clock::time_point wakeup) = 0;
};

#endif