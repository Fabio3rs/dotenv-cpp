#include "dotenv.hpp"
#include <benchmark/benchmark.h>
#include <filesystem>
#include <fstream>

#ifdef DOTENV_SIMD_ENABLED
#include "dotenv_mmap.hpp"
#include "dotenv_simd.hpp"

class AutoDetectionBenchmarkFixture : public benchmark::Fixture {
  public:
    void SetUp([[maybe_unused]] const ::benchmark::State &state) override {
        create_test_files();
    }

    void TearDown([[maybe_unused]] const ::benchmark::State &state) override {
        cleanup_test_files();
    }

  protected:
    void create_test_files() {
        // Arquivo muito pequeno (< 50KB, deve usar tradicional)
        create_test_file("auto_tiny.env", 20);

        // Arquivo médio (> 50KB, deve usar SIMD)
        create_test_file("auto_medium.env", 1000);

        // Arquivo grande (> 50KB, deve usar SIMD)
        create_test_file("auto_large.env", 10000);
    }

    static void create_test_file(const std::string &filename, int num_vars) {
        std::ofstream file(filename);

        for (int i = 0; i < num_vars; ++i) {
            file << "AUTO_VAR_" << i << "=auto_value_" << i << "\n";

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
        std::filesystem::remove("auto_tiny.env");
        std::filesystem::remove("auto_medium.env");
        std::filesystem::remove("auto_large.env");
    }
};

// Benchmark: Auto-detection load() for different file sizes
BENCHMARK_F(AutoDetectionBenchmarkFixture, AutoDetection_TinyFile)
(benchmark::State &state) {
    for (auto _ : state) {
        // Auto-detecção: deve usar tradicional (< 50KB)
        dotenv::load("auto_tiny.env", 1, false);
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(AutoDetectionBenchmarkFixture, AutoDetection_MediumFile)
(benchmark::State &state) {
    for (auto _ : state) {
        // Auto-detecção: deve usar SIMD (> 50KB)
        dotenv::load("auto_medium.env", 1, false);
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(AutoDetectionBenchmarkFixture, AutoDetection_LargeFile)
(benchmark::State &state) {
    for (auto _ : state) {
        // Auto-detecção: deve usar SIMD (> 50KB)
        dotenv::load("auto_large.env", 1, false);
    }
    state.SetItemsProcessed(state.iterations());
}

// Comparação: Auto-detection vs Forced Traditional vs Forced SIMD
BENCHMARK_F(AutoDetectionBenchmarkFixture, Comparison_AutoDetection_Large)
(benchmark::State &state) {
    for (auto _ : state) {
        dotenv::load("auto_large.env", 1, false);
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(AutoDetectionBenchmarkFixture, Comparison_Traditional_Large)
(benchmark::State &state) {
    for (auto _ : state) {
        dotenv::load_traditional("auto_large.env", 1, false);
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK_F(AutoDetectionBenchmarkFixture, Comparison_SIMD_Large)
(benchmark::State &state) {
    for (auto _ : state) {
        dotenv::load_simd("auto_large.env", 1, false);
    }
    state.SetItemsProcessed(state.iterations());
}

// Register benchmarks
BENCHMARK_REGISTER_F(AutoDetectionBenchmarkFixture, AutoDetection_TinyFile)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_REGISTER_F(AutoDetectionBenchmarkFixture, AutoDetection_MediumFile)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_REGISTER_F(AutoDetectionBenchmarkFixture, AutoDetection_LargeFile)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_REGISTER_F(AutoDetectionBenchmarkFixture,
                     Comparison_AutoDetection_Large)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_REGISTER_F(AutoDetectionBenchmarkFixture,
                     Comparison_Traditional_Large)
    ->Unit(benchmark::kMicrosecond);
BENCHMARK_REGISTER_F(AutoDetectionBenchmarkFixture, Comparison_SIMD_Large)
    ->Unit(benchmark::kMicrosecond);

#endif // DOTENV_SIMD_ENABLED
