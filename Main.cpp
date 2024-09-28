#include "Task.hpp"
#include "Executor.hpp"
#include "Fence.hpp"

#include <stdio.h>

Task<int> taskD(Executor &executor)
{
    printf("Task D beginning\n");
    co_await executor.yield();
    printf("Task D returning value\n");
    co_return 5;
}

Task<void> taskA(Executor &executor, Fence &fence)
{
    for(int i=0; i<5; i++) {
        printf("Task A (%i)\n", i);
        if(i == 3) {
            printf("...Task A await\n");
            co_await fence;
            printf("...Task A resume\n");
        } else {
            co_await executor.yield();
        }
    }

    printf("Task A awaiting from D\n");
    int result = co_await taskD(executor);
    printf("Task A received %i\n", result);

    co_return;
}

Task<void> taskB(Executor &executor, Fence &fence)
{
    for(int i=0; i<15; i++) {
        printf("Task B (%i)\n", i);
        if(i == 5) {
            printf("...Task B await\n");
            co_await fence;
            printf("...Task B resume\n");
        } else {
            co_await executor.yield();
        }
    }

    co_return;
}

Task<void> taskC(Executor &executor, Fence &fence)
{
    for(int i=0; i<15; i++) {
        printf("Task C (%i)\n", i);
        if(i == 10) {
            printf("...signal\n");
            fence.signal();
        }
        co_await executor.yield();
    }

    co_return;
}

int main(int argc, char *argv[])
{
    Executor executor;
    Fence fence(executor);

    executor.runAsync(taskA(executor, fence));
    executor.runAsync(taskB(executor, fence));
    executor.runAsync(taskC(executor, fence));

    executor.exec();

    return 0;
}