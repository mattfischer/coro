#include "Async.hpp"
#include "Task.hpp"
#include "Future.hpp"
#include "Actor.hpp"
#include "Executor.hpp"

#include <sstream>
#include <cstdarg>
#include <mutex>
#include <print>

static std::mutex tprintln_mutex;
template<typename ...Args> void tprintln(std::format_string<Args...> fmt, Args... args)
{
    std::lock_guard lock(tprintln_mutex);

    std::print("({}) ", std::this_thread::get_id());
    std::println(fmt, std::forward<Args...>(args)...);
}

Async<int> intReturnTask() {
    static int result = 5;
    tprintln("intReturnTask returning {}", result);
    co_return result++;
}

Async<int> taskD()
{
    tprintln("Task D beginning");
    co_await Task::yield();
    tprintln("Task D returning value");
    co_return 5;
}

Async<void> taskA(Actor &actor, Future<int> &future)
{
    int result = co_await actor.run(intReturnTask());
    tprintln("Task A received value {} from intReturnTask", result);

    for(int i=0; i<5; i++) {
        tprintln("Task A ({})", i);
        if(i == 3) {
            tprintln("...Task A await");
            result = co_await future;
            tprintln("...Task A resume (future returned {})", result);
        } else {
            co_await Task::yield();
        }
    }

    tprintln("Task A awaiting from D");
    result = co_await taskD();
    tprintln("Task A received {}", result);

    co_return;
}

Async<void> taskB(Actor &actor, Future<int> &future)
{
    int result = co_await actor.run(intReturnTask());
    tprintln("Task B received value {} from intReturnTask", result);

    for(int i=0; i<15; i++) {
        tprintln("Task B ({})", i);
        if(i == 5) {
            tprintln("...Task B await");
            result = co_await future;
            tprintln("...Task B resume (future returned {})", result);
        } else {
            co_await Task::yield();
        }
    }

    co_return;
}

Async<void> taskC(Actor &actor, Future<int> &future)
{
    int result = co_await actor.run(intReturnTask());
    tprintln("Task C received value {} from intReturnTask", result);

    using namespace std::chrono_literals;
 
    for(int i=0; i<15; i++) {
        tprintln("Task C ({})", i);
        if(i == 10) {
            tprintln("...complete future");
            future.complete(10);
        }
        co_await Task::yield();
    }

    tprintln("Task C sleeping...");
    co_await Task::sleep_for(1s);
    tprintln("...Task C done with sleep");

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