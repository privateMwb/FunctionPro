#pragma once

// Single include that pulls in the entire example framework — suite
// files include only this to get the library under test, example
// macros, output helpers, and suite registration.

// clang-format off
#include <FunctionPro/Function.h>            // FunctionPro::Function — SBO callable wrapper being showcased
#include <FunctionPro/MoveOnlyFunction.h>    // FunctionPro::MoveOnlyFunction — move-only variant being showcased
#include <FunctionPro/FunctionRef.h>         // FunctionPro::FunctionRef — non-owning callable view being showcased

#include "helpers.h"                 // output formatting & printing helpers
#include "macros.h"                  // REGISTER_EXAMPLE_SUITE()
#include "registry.h"                // ExampleSuite, ExampleRegistrar, example_registry()

#include <iomanip>                   // std::setw
// clang-format on

// ── Suite drivers ───────────────────────────────────────────────────
//
// These build on registry.h (example_registry()) and helpers.h
// (mainTitle(), setTitle()) — kept here rather than in helpers.h so
// that dependency only flows one way: this file already includes both
// before any of the below is defined.

// Prints the top-level banner once, then runs every registered suite
// in order, each under its own section title.
inline void printAllExampleSuite() {
    std::cout << "\n";

    for (const auto& suite : example_registry()) {
        std::cout << "\n";
        mainTitle(suite.name);
        suite.run();
    }

    std::cout << "\n";
}

// Prints every registered suite's id and name, grouped by category
// header, without running any of them.
inline void printExampleSuiteList() {
    std::cout << "\nAvailable example suites:\n";
    std::string category;

    for (const auto& suite : example_registry()) {
        if (category != suite.category)
            std::cout << '\n' << CYAN << prettify(suite.category) << RESET << '\n';
        std::cout << GREEN << std::left << std::setw(6) << ("[" + suite.id + "]") << RESET
                  << std::setw(30) << suite.name << '\n';
        category = suite.category;
    }

    std::cout << "\n";
}

// Runs a single suite under its own section title. No top-level
// banner — callers running a scoped subset don't need the "whole run"
// framing that printAllExampleSuite() prints.
inline void printOneSuite(const ExampleSuite& suite) {
    std::cout << "\n";
    mainTitle(suite.name);
    suite.run();
    std::cout << "\n";
}