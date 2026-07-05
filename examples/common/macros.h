// Registers this file's run_tests with the global registry so it runs
// automatically at startup. The anonymous namespace gives `registrar`
// internal linkage, preventing duplicate-symbol errors when this macro
// is expanded in multiple translation units.
#define REGISTER_EXAMPLE_SUITE()        \
    namespace {                      \
        static ExampleRegistrar registrar( \
            __FILE__,                \
            run_examples                \
        );                           \
    }