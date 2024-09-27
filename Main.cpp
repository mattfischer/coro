#include "Task.hpp"
#include "Executor.hpp"

#include <stdio.h>

Task::Fence fence;

Task runTaskA()
{
    for(int i=0; i<5; i++) {
        printf("Task A (%i)\n", i);
        if(i == 3) {
            printf("...await\n");
            co_await fence;
            printf("Task A resume\n");
        } else {
            co_yield nullptr;
        }
    }

    co_return;
}

Task runTaskB()
{
    for(int i=0; i<10; i++) {
        printf("Task B (%i)\n", i);
        if(i == 8) {
            printf("...signal\n");
            fence.signal();
        }
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