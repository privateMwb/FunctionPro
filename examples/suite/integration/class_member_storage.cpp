// Embedding callbacks as class members, across all three FunctionPro types.
//
// Demonstrates:
// - A class holding a configurable callback as a Function member
// - Setting the callback through a constructor and through a setter
// - Invoking the member callback from inside the class's own logic
// - The class remaining copyable because Function itself is copyable
// - A class holding a MoveOnlyFunction member, for a callback that owns
//   a resource and doesn't need to be copied
// - Why FunctionRef is passed as a parameter for one call rather than
//   stored as a member — and a class that does exactly that

#include <memory>
#include <string>
#include <support/framework.h>

using namespace FunctionPro;

// A button whose click behavior is configurable rather than hard-coded,
// stored directly as a Function member.
class Button {
  public:
    explicit Button(std::string label, Function<void()> onClick = {})
        : label_(std::move(label)), onClick_(std::move(onClick)) {}

    void setOnClick(Function<void()> callback) {
        onClick_ = std::move(callback);
    }

    void click() const {
        std::cout << "[" << label_ << "] clicked\n";
        if (onClick_) {
            onClick_();
        }
    }

  private:
    std::string label_;
    Function<void()> onClick_;
};

// A task whose completion handler owns a resource — a unique_ptr here —
// and only ever needs to run once. MoveOnlyFunction is the right member
// type: the task itself becomes move-only as a result, which is fine
// for something that represents in-flight work.
class UploadTask {
  public:
    explicit UploadTask(MoveOnlyFunction<void()> onComplete) : onComplete_(std::move(onComplete)) {}

    void finish() {
        if (onComplete_) {
            onComplete_();
            onComplete_.reset();
        }
    }

  private:
    MoveOnlyFunction<void()> onComplete_;
};

// FunctionRef isn't stored as a member here — a member field can easily
// outlive the caller's local lambda, which is exactly the dangling risk
// covered in misuse/dangling_functionref.cpp. Instead, it's accepted as
// a parameter and used only for the duration of a single call.
class ValidatingField {
  public:
    explicit ValidatingField(int value) : value_(value) {}

    bool validate(FunctionRef<bool(int)> rule) const {
        return rule(value_);
    }

  private:
    int value_;
};

static void run_examples() {

    // The callback can be supplied up front through the constructor,
    // just like any other member.
    setTitle("Setting the Callback at Construction");

    Button save("Save", [] { std::cout << "  -> saving document\n"; });

    save.click();
    std::cout << "\n";

    // A button built without a callback simply has an empty Function
    // member; click() already guards against calling it.
    setTitle("A Button Without a Callback");

    Button decorative("Decorative");

    decorative.click();
    std::cout << "\n";

    // The callback can also be attached later through a setter, since
    // Function supports move-assignment like any other member field.
    setTitle("Setting the Callback Later");

    int clickCount = 0;
    Button counter("Counter");
    counter.setOnClick([&clickCount] {
        ++clickCount;
        std::cout << "  -> click count is now " << clickCount << "\n";
    });

    counter.click();
    counter.click();
    std::cout << "\n";

    // Because Function is copyable, Button can be copied too — the copy
    // gets its own independent clone of the stored closure object. That
    // clone still captured clickCount by reference, so both buttons
    // affect the same counter; an independent counter would require the
    // lambda to capture by value instead.
    setTitle("Copying a Button Copies Its Callback");

    Button counterCopy = counter;
    counterCopy.click();

    std::cout << "clickCount after copy's click(): " << clickCount << "\n\n";

    // MoveOnlyFunction as a member lets the completion handler own a
    // resource the constructor captured — here, a unique_ptr wrapped
    // into the handler itself.
    setTitle("MoveOnlyFunction: A Member That Owns a Resource");

    auto reportName = std::make_unique<std::string>("upload-report.txt");
    UploadTask task([reportName = std::move(reportName)] {
        std::cout << "  -> wrote " << *reportName << "\n";
    });

    task.finish();
    task.finish(); // already fired once; onComplete_ was reset, so this is a no-op

    std::cout << "\n";

    // FunctionRef as a parameter, not a member, keeps the reference's
    // lifetime tied to the call — the local lambda below never needs to
    // outlive validate(), so there's nothing to dangle.
    setTitle("FunctionRef: Passed for One Call, Never Stored");

    ValidatingField age(17);
    auto isAdult = [](int v) { return v >= 18; };

    std::cout << "age.validate(isAdult): " << age.validate(isAdult) << "\n";
}

REGISTER_EXAMPLE_SUITE();
