#pragma once

#include "helper.h"

#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>

struct TestSuite {
    std::string id;
    std::string category;
    std::string name;
    void (*run)();
};

inline std::vector<TestSuite>& test_registry() {
    static std::vector<TestSuite> registry;
    return registry;
}

inline std::unordered_map<std::string, int>& category_counters() {
    static std::unordered_map<std::string, int> counters;
    return counters;
}

struct TestRegistrar {
    TestRegistrar(const char* file, void (*run)()) {
        auto path = std::filesystem::path(file);

        std::string category = path.parent_path().filename().string();

        char prefix = std::toupper(
            static_cast<unsigned char>(category.front())
        );

        int number = ++category_counters()[category];

        test_registry().push_back({
            std::string(1, prefix) + std::to_string(number),
            category,
            prettify(path.stem().string()),
            run
        });
    }
};