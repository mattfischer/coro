#include "Task.hpp"
#include "Executor.hpp"
#include "Fence.hpp"

#include <stdio.h>

Executor executor;
Fence fence(executor);

Task runTaskA()
{
    for(int i=0; i<5; i++) {
        printf("Task A (%i)\n", i);
        if(i == 3) {
            printf("...await\n");
            co_await fence;
            printf("Task A resume\n");
        } else {
            co_await executor.yield();
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
        co_await executor.yield();
    }

    co_return;
}

int main(int argc, char *argv[])
{
    Task taskA = runTaskA();
    Task taskB = runTaskB();

    executor.queue(taskA.handle());
    executor.queue(taskB.handle());

    executor.run();

    return 0;
}