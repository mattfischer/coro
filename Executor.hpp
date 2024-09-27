#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include "Task.hpp"

#include <vector>

class Executor {
    public:
        void addTask(Task task);

        void run();

    private:
        std::vector<Task> mTasks;
};

#endif