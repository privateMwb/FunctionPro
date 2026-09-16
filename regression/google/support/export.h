#pragma once

// clang-format off
#include "helpers.h"   // regression_results()
                       // markdown_buffer(), custom

#include <fstream>     // std::ofstream (exportJson, exportMarkdown)
#include <iomanip>     // std::fixed, std::setprecision (exportJson)
#include <string>      // std::string (filename params)
// clang-format on

constexpr const char* kResultsDir = "regression/results/";

// Writes regression_results() out as a JSON array to <kResultsDir>/filename.
// One entry per RegressionRow -- flat by benchmark name, since Google
// Benchmark's own snapshots carry no suite/operation split to preserve.
inline void exportJson(const std::string& filename) {
    std::ofstream out(kResultsDir + filename);

    if (!out)
        return;

    out << "[\n";

    const auto& results = regression_results();

    for (std::size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];

        out << "  {\n";
        out << "    \"name\": \"" << r.name << "\",\n";
        out << "    \"baseline_ns\": " << std::fixed << std::setprecision(2) << r.baseline_ns
            << ",\n";
        out << "    \"current_ns\": " << std::fixed << std::setprecision(2) << r.current_ns
            << ",\n";
        out << "    \"pct_change\": " << std::fixed << std::setprecision(2) << r.pct_change << "\n";
        out << "  }";

        if (i + 1 != results.size())
            out << ",";

        out << "\n";
    }

    out << "]\n";
}

// Writes the markdown transcript accumulated in markdown_buffer() (built up
// by setHeader()/printComparisonRow() during the run) to a file.
inline void exportMarkdown(const std::string& filename) {
    std::ofstream out(kResultsDir + filename);

    if (!out)
        return;

    out << "#" << custom << "Regression Report\n";
    out << markdown_buffer();
}
