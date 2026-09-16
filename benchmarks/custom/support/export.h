#pragma once

// clang-format off
#include "registry.h"       // benchmark_results()
#include "helpers.h"        // custom (comparison label set by setCustom())
                            // markdown_buffer()

#include <fstream>          // std::ofstream
#include <iomanip>          // std::fixed, std::setprecision
#include <string>           // std::string
// clang-format on

// All export output lands here. Caller passes just a filename
// ("benchmark_results.json"), not a path — this prefixes it. Note this
// does NOT create the directory if it's missing; benchmarks/results/ must
// already exist in the repo (or be created by CI before this runs), or
// ofstream silently fails to open and these functions no-op.
constexpr const char* kResultsDir = "benchmarks/results/";

// Writes every recorded result in benchmark_results() to a JSON array,
// one object per result. Used by the regression tool to compare runs
// across commits.
inline void exportJson(const std::string& filename) {
    std::ofstream out(kResultsDir + filename);

    if (!out)
        return;

    out << "[\n";

    const auto& results = benchmark_results();

    for (std::size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];

        out << "  {\n";
        out << "    \"suite\": \"" << r.suite << "\",\n";
        out << "    \"operation\": \"" << r.operation << "\",\n";
        out << "    \"total_ns\": " << r.total_ns << ",\n";
        out << "    \"iterations\": " << r.iterations << ",\n";
        out << "    \"ns_per_op\": " << std::fixed << std::setprecision(2) << r.ns_per_op << "\n";
        out << "  }";

        if (i + 1 != results.size())
            out << ",";

        out << "\n";
    }

    out << "]\n";
}

// Writes the markdown transcript accumulated in markdown_buffer() (built up
// by setHeader()/printComparisonRow() during the run) to a file. Unlike
// exportJson(), this can't be derived from benchmark_results() after the
// fact — it needs the section grouping and paired rows that only exist at
// print time.
inline void exportMarkdown(const std::string& filename) {
    std::ofstream out(kResultsDir + filename);

    if (!out)
        return;

    out << "# " << custom << "Benchmark Results\n";
    out << markdown_buffer();
}