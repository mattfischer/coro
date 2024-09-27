#include "Executor.hpp"

void Executor::addTask(Task task)
{
    mTasks.push_back(std::move(task));
}

void Executor::run()
{
    while(mTasks.size() > 0) {
        for(int i=0; i<mTasks.size(); i++) {
            if(mTasks[i].done()) {
                mTasks.erase(mTasks.begin() + i);
                i--;
            } else if(mTasks[i].runnable()) {
                mTasks[i].run();
            }
        }
    }
}