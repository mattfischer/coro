#include "Async.hpp"
#include "Task.hpp"
#include "Future.hpp"

#include <stdio.h>

Async<int> taskD()
{
    printf("Task D beginning\n");
    co_await Task::yield();
    printf("Task D returning value\n");
    co_return 5;
}

Async<void> taskA(Future<int> &future)
{
    for(int i=0; i<5; i++) {
        printf("Task A (%i)\n", i);
        if(i == 3) {
            printf("...Task A await\n");
            int result = co_await future;
            printf("...Task A resume (future returned %i)\n", result);
        } else {
            co_await Task::yield();
        }
    }

    printf("Task A awaiting from D\n");
    int result = co_await taskD();
    printf("Task A received %i\n", result);

    co_return;
}

Async<void> taskB(Future<int> &future)
{
    for(int i=0; i<15; i++) {
        printf("Task B (%i)\n", i);
        if(i == 5) {
            printf("...Task B await\n");
            int result = co_await future;
            printf("...Task B resume (future returned %i)\n", result);
        } else {
            co_await Task::yield();
        }
    }

    co_return;
}

Async<void> taskC(Future<int> &future)
{
    using namespace std::chrono_literals;
 
    for(int i=0; i<15; i++) {
        printf("Task C (%i)\n", i);
        if(i == 10) {
            printf("...complete future\n");
            future.complete(10);
        }
        co_await Task::yield();
    }

    printf("Task C sleeping...\n");
    co_await Task::sleep_for(1s);
    printf("...Task C done with sleep\n");

    co_return;
}

int main(int argc, char *argv[])
{
    Executor executor;
    Future<int> future;

    Task::start(taskA(future), executor);
    Task::start(taskB(future), executor);
    Task::start(taskC(future), executor);

    executor.exec();

    return 0;
}