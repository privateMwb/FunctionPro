// Storing heterogeneous callbacks in a registry, all three ways.
//
// Demonstrates:
// - A map of named callbacks, each an independent Function
// - Registering callbacks with unrelated capture shapes under one type
// - Looking up and invoking a callback by name
// - Replacing a registered callback without disturbing the others
// - A registry of MoveOnlyFunction handlers that fire once and erase
//   themselves
// - Why FunctionRef doesn't belong in a persistent registry, and what
//   it's used for instead: a one-off, same-scope batch dispatch

#include <map>
#include <string>
#include <support/framework.h>

using namespace FunctionPro;

// A simple named-callback registry: any Function<void(const std::string&)>
// can be registered under a name and invoked later by that name.
class EventRegistry {
  public:
    void on(const std::string& name, Function<void(const std::string&)> handler) {
        handlers_[name] = std::move(handler);
    }

    void fire(const std::string& name, const std::string& payload) const {
        auto it = handlers_.find(name);
        if (it != handlers_.end() && it->second) {
            it->second(payload);
        } else {
            std::cout << "no handler registered for \"" << name << "\"\n";
        }
    }

  private:
    std::map<std::string, Function<void(const std::string&)>> handlers_;
};

// A registry where each handler runs at most once, then is erased — a
// good fit for MoveOnlyFunction, since a fired handler is never called
// or copied again, and may own resources that only it needs.
class OnceRegistry {
  public:
    void on(const std::string& name, MoveOnlyFunction<void(const std::string&)> handler) {
        handlers_[name] = std::move(handler);
    }

    void fire(const std::string& name, const std::string& payload) {
        auto it = handlers_.find(name);
        if (it != handlers_.end()) {
            it->second(payload);
            handlers_.erase(it);
        }
    }

  private:
    std::map<std::string, MoveOnlyFunction<void(const std::string&)>> handlers_;
};

// FunctionRef isn't suitable for a registry that outlives the call that
// populated it — the handlers here would dangle the moment their local
// lambdas went out of scope (see misuse/dangling_functionref.cpp). What
// it's suited for is a table built and consumed within one call, like
// this batch dispatch.
static void
dispatchBatch(const std::map<std::string, FunctionRef<void(const std::string&)>>& handlers,
              const std::string& name, const std::string& payload) {
    auto it = handlers.find(name);
    if (it != handlers.end()) {
        it->second(payload);
    }
}

static void run_examples() {

    // Every entry in the registry is a Function with the same signature,
    // even though each callback's captures are completely unrelated.
    setTitle("Function: Registering Callbacks");

    EventRegistry events;

    events.on("startup", [](const std::string& msg) { std::cout << "startup: " << msg << "\n"; });

    int errorCount = 0;
    events.on("error", [&errorCount](const std::string& msg) {
        ++errorCount;
        std::cout << "error #" << errorCount << ": " << msg << "\n";
    });

    std::cout << "handlers registered\n\n";

    // Firing looks the handler up by name and invokes it with the
    // supplied payload, the same way regardless of what it captured.
    setTitle("Function: Firing Events");

    events.fire("startup", "service ready");
    events.fire("error", "connection refused");
    events.fire("error", "timeout");
    std::cout << "\n";

    // A name with no registered handler is handled explicitly rather
    // than silently doing nothing.
    setTitle("Function: Firing an Unregistered Event");

    events.fire("shutdown", "graceful");
    std::cout << "\n";

    // Re-registering a name replaces only that entry — every other
    // handler in the registry is untouched.
    setTitle("Function: Replacing a Handler");

    events.on("startup",
              [](const std::string& msg) { std::cout << "startup (v2): " << msg << "\n"; });

    events.fire("startup", "service ready");
    events.fire("error", "still counting");
    std::cout << "\n";

    // MoveOnlyFunction handlers fire at most once — after that, the
    // entry is gone from the map entirely, not just left empty.
    setTitle("MoveOnlyFunction: Fire-Once Handlers");

    OnceRegistry onceEvents;

    onceEvents.on("init", [](const std::string& msg) { std::cout << "init: " << msg << "\n"; });

    onceEvents.fire("init", "loading config");
    onceEvents.fire("init", "loading config"); // already erased; this does nothing

    std::cout << "\n";

    // A FunctionRef table built and consumed within this one call —
    // nothing here is stored past dispatchBatch() returning.
    setTitle("FunctionRef: A One-Off Batch Dispatch");

    auto onOpen = [](const std::string& msg) { std::cout << "open: " << msg << "\n"; };
    auto onClose = [](const std::string& msg) { std::cout << "close: " << msg << "\n"; };

    std::map<std::string, FunctionRef<void(const std::string&)>> batch{
        {"open", onOpen},
        {"close", onClose},
    };

    dispatchBatch(batch, "open", "connection 7");
    dispatchBatch(batch, "close", "connection 7");
}

REGISTER_EXAMPLE_SUITE();
