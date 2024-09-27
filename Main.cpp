#include "Task.hpp"
#include "Executor.hpp"

#include <stdio.h>

Task runTaskA()
{
    for(int i=0; i<5; i++) {
        printf("Task A\n");
        co_yield nullptr;
    }

    co_return;
}

Task runTaskB()
{
    for(int i=0; i<6; i++) {
        printf("Task B\n");
        co_yield nullptr;
    }

    co_return;
}

int main(int argc, char *argv[])
{
    Task taskA = runTaskA();
    Task taskB = runTaskB();

    Executor executor;
    executor.addTask(std::move(taskA));
    executor.addTask(std::move(taskB));

    executor.run();

    return 0;
}