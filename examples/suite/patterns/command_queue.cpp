// Queuing tasks with all three FunctionPro types.
//
// Demonstrates:
// - A queue of move-only tasks, each owning its own resources
// - Tasks that capture a std::unique_ptr, which Function could not hold
// - Draining the queue in submission order
// - A queue of copyable Function tasks, where the same task can be
//   requeued after running
// - FunctionRef used for a batch that's processed immediately, never
//   queued for later — the pattern it's actually suited for here

#include <memory>
#include <string>
#include <support/framework.h>
#include <vector>

using namespace FunctionPro;

static void run_examples() {

    // A command queue is just a container of pending work.
    // MoveOnlyFunction is the right tool when each task is enqueued
    // once, run once, and never needs to be copied.
    setTitle("MoveOnlyFunction: Building a Queue");

    std::vector<MoveOnlyFunction<void()>> queue;

    queue.push_back([] { std::cout << "task 1: flush logs\n"; });
    queue.push_back([] { std::cout << "task 2: rotate cache\n"; });

    std::cout << "queued tasks: " << queue.size() << "\n\n";

    // A task can own resources that only it needs — here, a unique_ptr
    // that Function's copy requirement would have rejected outright.
    setTitle("MoveOnlyFunction: A Task Owning a Resource");

    auto payload = std::make_unique<std::string>("pending write");

    queue.push_back([payload = std::move(payload)]() mutable {
        std::cout << "task 3: writing \"" << *payload << "\"\n";
    });

    std::cout << "queued tasks: " << queue.size() << "\n\n";

    // Draining the queue runs each task exactly once, in the order it was
    // submitted, then discards it.
    setTitle("MoveOnlyFunction: Draining the Queue");

    for (auto& task : queue) {
        task();
    }

    queue.clear();

    std::cout << "\nqueued tasks after drain: " << queue.size() << "\n\n";

    // Function is the right tool when a task might need to be requeued
    // or handed to more than one consumer — copying it just duplicates
    // the queued work, unlike MoveOnlyFunction which can only transfer it.
    setTitle("Function: A Requeueable Task");

    std::vector<Function<void()>> retryQueue;
    Function<void()> heartbeat = [] { std::cout << "heartbeat\n"; };

    retryQueue.push_back(heartbeat); // copy: the original is still usable
    retryQueue.push_back(heartbeat); // queued a second time, independently

    for (auto& task : retryQueue) {
        task();
    }

    std::cout << "\n";

    // FunctionRef doesn't belong in a persisted queue — the referenced
    // callables here are local variables that go out of scope at the end
    // of run_examples(), and a FunctionRef stored past that point would
    // dangle (see misuse/dangling_functionref.cpp). What it's suited for
    // is processing a batch of callbacks immediately, in the same scope
    // they were declared in, without taking ownership of any of them.
    setTitle("FunctionRef: An Immediate Batch, Not a Stored Queue");

    auto step1 = [] { std::cout << "step 1: validate\n"; };
    auto step2 = [] { std::cout << "step 2: commit\n"; };

    std::vector<FunctionRef<void()>> batch{step1, step2};

    for (auto step : batch) {
        step();
    }
}

REGISTER_EXAMPLE_SUITE();
