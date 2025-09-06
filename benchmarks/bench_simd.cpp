#include "dotenv.hpp"
#include <benchmark/benchmark.h>
#include <filesystem>
#include <fstream>

#ifdef DOTENV_SIMD_ENABLED
#include "dotenv_mmap.hpp"
#include "dotenv_simd.hpp"

class SIMDBenchmarkFixture : public benchmark::Fixture {
  public:
    void SetUp([[maybe_unused]] const ::benchmark::State &state) override {
        create_test_files();
    }

    void TearDown([[maybe_unused]] const ::benchmark::State &state) override {
        cleanup_test_files();
    }

  protected:
    void create_test_files() {
        // Small file (100 lines)
        create_test_file("simd_small.env", 100);

        // Medium file (1000 lines)
        create_test_file("simd_medium.env", 1000);

        // Large file (10000 lines)
        create_test_file("simd_large.env", 10000);

        // Very large file (100000 lines)
        create_test_file("simd_xlarge.env", 100000);
    }

    static void create_test_file(const std::string &filename, int num_vars) {
        std::ofstream file(filename);

        for (int i = 0; i < num_vars; ++i) {
            file << "SIMD_VAR_" << i << "=simd_value_" << i << "\n";

            // Add some comments and empty lines for realism
            if (i % 10 == 0) {
                file << "# Comment line " << i << "\n";
            }
            if (i % 20 == 0) {
                file << "\n"; // Empty line
            }
        }
    }

    static void cleanup_test_files() {
        std::filesystem::remove("simd_small.env");
        std::filesystem::remove("simd_medium.env");
        std::filesystem::remove("simd_large.env");
        std::filesystem::remove("simd_xlarge.env");
    }
};

// Benchmark: Load Small File - Standard vs SIMD
BENCHMARK_F(SIMDBenchmarkFixture, LoadSmallFile_Standard)
(benchmark::State &state) {
    for (auto _ : state) {
        // Força implementação tradicional (sem SIMD)
        dotenv::load_traditional("simd_small.env", 1, false);
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(SIMDBenchmarkFixture, LoadSmallFile_SIMD)(benchmark::State &state) {
    for (auto _ : state) {
        // Força SIMD independentemente do tamanho
        dotenv::load_simd("simd_small.env", 1, false);
    }
    state.SetItemsProcessed(state.iterations());
}

// Benchmark: Load Medium File - Standard vs SIMD
BENCHMARK_F(SIMDBenchmarkFixture, LoadMediumFile_Standard)
(benchmark::State &state) {
    for (auto _ : state) {
        // Força implementação tradicional (sem SIMD)
        dotenv::load_traditional("simd_medium.env", 1, false);
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(SIMDBenchmarkFixture, LoadMediumFile_SIMD)
(benchmark::State &state) {
    for (auto _ : state) {
        // Força SIMD independentemente do tamanho
        dotenv::load_simd("simd_medium.env", 1, false);
    }
    state.SetItemsProcessed(state.iterations());
}

// Benchmark: Load Large File - Standard vs SIMD
BENCHMARK_F(SIMDBenchmarkFixture, LoadLargeFile_Standard)
(benchmark::State &state) {
    for (auto _ : state) {
        // Força implementação tradicional (sem SIMD)
        dotenv::load_traditional("simd_large.env", 1, false);
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(SIMDBenchmarkFixture, LoadLargeFile_SIMD)(benchmark::State &state) {
    for (auto _ : state) {
        // Força SIMD independentemente do tamanho
        dotenv::load_simd("simd_large.env", 1, false);
    }
    state.SetItemsProcessed(state.iterations());
}

// Benchmark: Load Very Large File - Standard vs SIMD
BENCHMARK_F(SIMDBenchmarkFixture, LoadXLargeFile_Standard)
(benchmark::State &state) {
    for (auto _ : state) {
        // Força implementação tradicional (sem SIMD)
        dotenv::load_traditional("simd_xlarge.env", 1, false);
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(SIMDBenchmarkFixture, LoadXLargeFile_SIMD)
(benchmark::State &state) {
    for (auto _ : state) {
        // Força SIMD independentemente do tamanho
        dotenv::load_simd("simd_xlarge.env", 1, false);
    }
    state.SetItemsProcessed(state.iterations());
}

// Callback vs Vector comparison - demonstra superioridade do callback
BENCHMARK_F(SIMDBenchmarkFixture, ProcessLines_Callback_Large)
(benchmark::State &state) {
    dotenv::mapped_file mmap_file("simd_large.env");
    auto content = mmap_file.view();

    for (auto _ : state) {
        size_t count = 0;
        auto result = dotenv::simd::process_lines_avx2(
            content, '\n', [&count](size_t, std::string_view) { ++count; });
        benchmark::DoNotOptimize(count);
        benchmark::DoNotOptimize(result);
    }
}

// Register benchmarks with timing units
BENCHMARK_REGISTER_F(SIMDBenchmarkFixture, LoadSmallFile_Standard)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_REGISTER_F(SIMDBenchmarkFixture, LoadSmallFile_SIMD)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_REGISTER_F(SIMDBenchmarkFixture, LoadMediumFile_Standard)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_REGISTER_F(SIMDBenchmarkFixture, LoadMediumFile_SIMD)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_REGISTER_F(SIMDBenchmarkFixture, LoadLargeFile_Standard)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_REGISTER_F(SIMDBenchmarkFixture, LoadLargeFile_SIMD)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_REGISTER_F(SIMDBenchmarkFixture, LoadXLargeFile_Standard)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_REGISTER_F(SIMDBenchmarkFixture, LoadXLargeFile_SIMD)
    ->Unit(benchmark::kMillisecond);

#endif // DOTENV_SIMD_ENABLED
