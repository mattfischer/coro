#include "Task.hpp"
#include "Executor.hpp"
#include "Future.hpp"

#include <stdio.h>

Task<int> taskD(Executor &executor)
{
    printf("Task D beginning\n");
    co_await executor.yield();
    printf("Task D returning value\n");
    co_return 5;
}

Task<void> taskA(Executor &executor, Future<int> &future)
{
    for(int i=0; i<5; i++) {
        printf("Task A (%i)\n", i);
        if(i == 3) {
            printf("...Task A await\n");
            int result = co_await future;
            printf("...Task A resume (future returned %i)\n", result);
        } else {
            co_await executor.yield();
        }
    }

    printf("Task A awaiting from D\n");
    int result = co_await taskD(executor);
    printf("Task A received %i\n", result);

    co_return;
}

Task<void> taskB(Executor &executor, Future<int> &future)
{
    for(int i=0; i<15; i++) {
        printf("Task B (%i)\n", i);
        if(i == 5) {
            printf("...Task B await\n");
            int result = co_await future;
            printf("...Task B resume (future returned %i)\n", result);
        } else {
            co_await executor.yield();
        }
    }

    co_return;
}

Task<void> taskC(Executor &executor, Future<int> &future)
{
    using namespace std::chrono_literals;
 
    for(int i=0; i<15; i++) {
        printf("Task C (%i)\n", i);
        if(i == 10) {
            printf("...complete future\n");
            future.complete(10);
        }
        co_await executor.yield();
    }

    printf("Task C sleeping...\n");
    co_await executor.sleep_for(1s);
    printf("...Task C done with sleep\n");

    co_return;
}

int main(int argc, char *argv[])
{
    Executor executor;
    Future<int> future(executor);

    executor.runAsync(taskA(executor, future));
    executor.runAsync(taskB(executor, future));
    executor.runAsync(taskC(executor, future));

    executor.exec();

    return 0;
}