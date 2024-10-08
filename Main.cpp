#include "Async.hpp"
#include "Task.hpp"
#include "Future.hpp"
#include "Actor.hpp"
#include "ExecutorSerial.hpp"

#include <stdio.h>

Async<int> intReturnTask() {
    static int result = 5;
    printf("intReturnTask returning %i\n", result);
    co_return result++;
}

Async<int> taskD()
{
    printf("Task D beginning\n");
    co_await Task::yield();
    printf("Task D returning value\n");
    co_return 5;
}

Async<void> taskA(Actor &actor, Future<int> &future)
{
    int result = co_await actor.run(intReturnTask());
    printf("Task A received value %i from intReturnTask\n", result);

    for(int i=0; i<5; i++) {
        printf("Task A (%i)\n", i);
        if(i == 3) {
            printf("...Task A await\n");
            result = co_await future;
            printf("...Task A resume (future returned %i)\n", result);
        } else {
            co_await Task::yield();
        }
    }

    printf("Task A awaiting from D\n");
    result = co_await taskD();
    printf("Task A received %i\n", result);

    co_return;
}

Async<void> taskB(Actor &actor, Future<int> &future)
{
    int result = co_await actor.run(intReturnTask());
    printf("Task B received value %i from intReturnTask\n", result);

    for(int i=0; i<15; i++) {
        printf("Task B (%i)\n", i);
        if(i == 5) {
            printf("...Task B await\n");
            result = co_await future;
            printf("...Task B resume (future returned %i)\n", result);
        } else {
            co_await Task::yield();
        }
    }

    co_return;
}

Async<void> taskC(Actor &actor, Future<int> &future)
{
    int result = co_await actor.run(intReturnTask());
    printf("Task C received value %i from intReturnTask\n", result);

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
    ExecutorSerial executor;
    Future<int> future;
    Actor actor(executor);

    Task::start(taskA(actor, future), executor);
    Task::start(taskB(actor, future), executor);
    Task::start(taskC(actor, future), executor);

    executor.exec();

    return 0;
}