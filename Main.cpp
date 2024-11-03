#include "Async.hpp"
#include "Task.hpp"
#include "Future.hpp"
#include "Actor.hpp"
#include "Executor.hpp"

#include <print>

template<typename ...Args> void log(std::format_string<Args...> fmt, Args... args)
{
    std::println("[ {:6} ]  {}", std::this_thread::get_id(), std::format(fmt, std::forward<Args...>(args)...));
}

Async<int> intReturnFunc() {
    static int result = 5;
    log("intReturnFunc returning {}", result);
    co_return result++;
}

Async<int> subFunc()
{
    log("subFunc beginning");
    co_await Task::yield();
    log("subFunc returning value");
    co_return 5;
}

Async<void> taskA(Actor &actor, Future<int> &future)
{
    int result = co_await actor.run(intReturnFunc());
    log("Task A received value {} from intReturnFunc", result);

    for(int i=0; i<5; i++) {
        log("Task A ({})", i);
        if(i == 3) {
            log("...Task A await");
            result = co_await future;
            log("...Task A resume (future returned {})", result);
        } else {
            co_await Task::yield();
        }
    }

    log("Task A awaiting from subFunc");
    result = co_await subFunc();
    log("Task A received {} from subFunc", result);

    co_return;
}

Async<void> taskB(Actor &actor, Future<int> &future)
{
    int result = co_await actor.run(intReturnFunc());
    log("Task B received value {} from intReturnFunc", result);

    for(int i=0; i<15; i++) {
        log("Task B ({})", i);
        if(i == 5) {
            log("...Task B await");
            result = co_await future;
            log("...Task B resume (future returned {})", result);
        } else {
            co_await Task::yield();
        }
    }

    co_return;
}

Async<void> taskC(Actor &actor, Future<int> &future)
{
    int result = co_await actor.run(intReturnFunc());
    log("Task C received value {} from intReturnFunc", result);

    using namespace std::chrono_literals;
 
    for(int i=0; i<15; i++) {
        log("Task C ({})", i);
        if(i == 10) {
            log("...complete future");
            future.complete(10);
        }
        co_await Task::yield();
    }

    log("Task C sleeping...");
    co_await Task::sleep_for(1s);
    log("...Task C done with sleep");

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

    Task::start(
        [&]() -> Async<void> {
            co_await Task::sleep_for(std::chrono::seconds(5));
            executor.stop();
        }(),
        executor
    );

    executor.start(2, true);

    return 0;
}