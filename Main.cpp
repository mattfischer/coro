#include "Task.hpp"
#include "Executor.hpp"
#include "Fence.hpp"

#include <stdio.h>

Task<int> runTaskD(Executor &executor)
{
    printf("Task D beginning\n");
    co_await executor.yield();
    printf("Task D returning value\n");
    co_return 5;
}

Task<void> runTaskA(Executor &executor, Fence &fence)
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
    int result = co_await runTaskD(executor);
    printf("Task A received %i\n", result);

    co_return;
}

Task<void> runTaskB(Executor &executor, Fence &fence)
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

Task<void> runTaskC(Executor &executor, Fence &fence)
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

    Task taskA = runTaskA(executor, fence);
    Task taskB = runTaskB(executor, fence);
    Task taskC = runTaskC(executor, fence);

    executor.queue(taskA.handle());
    executor.queue(taskB.handle());
    executor.queue(taskC.handle());

    executor.run();

    return 0;
}