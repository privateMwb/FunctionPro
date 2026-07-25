// Subscribing to and notifying callbacks, across all three FunctionPro types.
//
// Demonstrates:
// - A subject holding a list of subscriber callbacks
// - Subscribing observers with different capture shapes
// - Notifying every subscriber on an event
// - Unsubscribing by clearing a stored callback
// - A one-shot subscription using MoveOnlyFunction, removed after firing
// - FunctionRef fanning out to observers immediately, without a subject
//   that stores them at all

#include <memory>
#include <string>
#include <support/framework.h>
#include <vector>

using namespace FunctionPro;

// A minimal subject: anyone can subscribe a callback, and every
// subscriber is notified when an event fires.
class TemperatureSensor {
  public:
    void subscribe(Function<void(double)> callback) {
        observers_.push_back(std::move(callback));
    }

    void notify(double celsius) const {
        for (const auto& observer : observers_) {
            if (observer) {
                observer(celsius);
            }
        }
    }

    void unsubscribeAt(std::size_t index) {
        if (index < observers_.size()) {
            observers_[index].reset();
        }
    }

  private:
    std::vector<Function<void(double)>> observers_;
};

// A subject where each subscriber fires at most once, then is dropped —
// suited to MoveOnlyFunction, since a fired callback never needs to be
// copied or called again.
class OneShotAlarm {
  public:
    void subscribe(MoveOnlyFunction<void(double)> callback) {
        observer_ = std::move(callback);
    }

    void trigger(double celsius) {
        if (observer_) {
            observer_(celsius);
            observer_.reset();
        }
    }

  private:
    MoveOnlyFunction<void(double)> observer_;
};

static void run_examples() {

    // Subscribers can be lambdas with their own captures — here, a label
    // baked in at subscription time.
    setTitle("Function: Subscribing Observers");

    TemperatureSensor sensor;

    sensor.subscribe([](double c) { std::cout << "display: " << c << "C\n"; });

    std::string logPrefix = "log:";
    sensor.subscribe([logPrefix](double c) { std::cout << logPrefix << " reading " << c << "\n"; });

    std::cout << "observers subscribed\n\n";

    // notify() calls every currently-held observer with the same event
    // data, in subscription order.
    setTitle("Function: Notifying Observers");

    sensor.notify(21.5);

    std::cout << "\n";

    // Unsubscribing just resets that slot's Function to empty; notify()
    // already skips empty entries, so the observer stops firing without
    // needing to compact the list.
    setTitle("Function: Unsubscribing");

    sensor.unsubscribeAt(0);
    sensor.notify(22.0);
    std::cout << "\n";

    // MoveOnlyFunction fits a subscriber that owns a resource and should
    // only ever fire once — the alarm resets itself after triggering,
    // and a second trigger is silently a no-op.
    setTitle("MoveOnlyFunction: A One-Shot Subscriber");

    OneShotAlarm alarm;

    auto label = std::make_unique<std::string>("overheat");
    alarm.subscribe(
        [label = std::move(label)](double c) { std::cout << *label << ": " << c << "C\n"; });

    alarm.trigger(95.0);
    alarm.trigger(96.0); // already fired once; this does nothing

    std::cout << "\n";

    // FunctionRef doesn't belong as a stored subscriber (see
    // patterns/command_queue.cpp for the same reasoning) — but fanning
    // an event out to a batch of callbacks declared in the same scope,
    // with no subject object involved at all, is exactly what it's for.
    setTitle("FunctionRef: Fanning Out Without a Stored Subject");

    auto toConsole = [](double c) { std::cout << "console: " << c << "C\n"; };
    auto toFile = [](double c) { std::cout << "file: " << c << "C\n"; };

    for (FunctionRef<void(double)> sink :
         {FunctionRef<void(double)>(toConsole), FunctionRef<void(double)>(toFile)}) {
        sink(23.0);
    }
}

REGISTER_EXAMPLE_SUITE();
