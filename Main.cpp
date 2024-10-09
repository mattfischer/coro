#include "Async.hpp"
#include "Task.hpp"
#include "Future.hpp"
#include "Actor.hpp"
#include "Executor.hpp"

#include <sstream>
#include <cstdarg>
#include <mutex>
#include <stdio.h>

void tprintf(const char *fmt, ...)
{
    static std::mutex mutex;
    std::lock_guard lock(mutex);

    std::stringstream ss;
    ss << std::this_thread::get_id();
    printf("(%s) ", ss.str().c_str());

    std::va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
}

Async<int> intReturnTask() {
    static int result = 5;
    tprintf("intReturnTask returning %i\n", result);
    co_return result++;
}

Async<int> taskD()
{
    tprintf("Task D beginning\n");
    co_await Task::yield();
    tprintf("Task D returning value\n");
    co_return 5;
}

Async<void> taskA(Actor &actor, Future<int> &future)
{
    int result = co_await actor.run(intReturnTask());
    tprintf("Task A received value %i from intReturnTask\n", result);

    for(int i=0; i<5; i++) {
        tprintf("Task A (%i)\n", i);
        if(i == 3) {
            tprintf("...Task A await\n");
            result = co_await future;
            tprintf("...Task A resume (future returned %i)\n", result);
        } else {
            co_await Task::yield();
        }
    }

    tprintf("Task A awaiting from D\n");
    result = co_await taskD();
    tprintf("Task A received %i\n", result);

    co_return;
}

Async<void> taskB(Actor &actor, Future<int> &future)
{
    int result = co_await actor.run(intReturnTask());
    tprintf("Task B received value %i from intReturnTask\n", result);

    for(int i=0; i<15; i++) {
        tprintf("Task B (%i)\n", i);
        if(i == 5) {
            tprintf("...Task B await\n");
            result = co_await future;
            tprintf("...Task B resume (future returned %i)\n", result);
        } else {
            co_await Task::yield();
        }
    }

    co_return;
}

Async<void> taskC(Actor &actor, Future<int> &future)
{
    int result = co_await actor.run(intReturnTask());
    tprintf("Task C received value %i from intReturnTask\n", result);

    using namespace std::chrono_literals;
 
    for(int i=0; i<15; i++) {
        tprintf("Task C (%i)\n", i);
        if(i == 10) {
            tprintf("...complete future\n");
            future.complete(10);
        }
        co_await Task::yield();
    }

    tprintf("Task C sleeping...\n");
    co_await Task::sleep_for(1s);
    tprintf("...Task C done with sleep\n");

    co_return;
}

int main(int argc, char *argv[])
{
    Executor executor;
    Future<int> future;
    Actor actor(executor);

    Task::start(taskA(actor, future), executor);
    Task::start(taskB(actor, future), executor);
    Task::start(taskC(actor, future), executor);

    executor.start(2);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    executor.stop();

    return 0;
}