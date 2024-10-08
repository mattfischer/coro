#include "Actor.hpp"

Actor::Actor(Executor &executor) {
    Task::start(runLoop(), executor);
}

Async<void> Actor::runLoop() {
    while(true) {
        RunItem *item = co_await mQueue;
        co_await item->run();
    }
    co_return;
}
