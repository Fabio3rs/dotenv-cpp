#include "dotenv.hpp"
#include <benchmark/benchmark.h>

#ifdef DOTENV_SIMD_ENABLED
#include "dotenv_mmap.hpp"
#include "dotenv_simd.hpp"
#endif

#include <filesystem>
#include <fstream>

namespace {

constexpr size_t SMALL_FILE_LINES = 100;
constexpr size_t MEDIUM_FILE_LINES = 10000;
constexpr size_t LARGE_FILE_LINES = 100000;

const std::string test_file = "benchmark_test.env";

// Generate simple test .env file
void generate_test_file(size_t num_lines) {
    std::ofstream file(test_file);
    for (size_t i = 0; i < num_lines; ++i) {
        file << "KEY_" << i << "=value_" << i << "_some_longer_content_here\n";
    }
}

} // anonymous namespace

// Benchmark traditional loading
static void BM_Traditional_Small(benchmark::State &bench_state) {
    generate_test_file(SMALL_FILE_LINES);

    for ([[maybe_unused]] auto iteration : bench_state) {
        auto result = dotenv::load_traditional(test_file, 1, false);
        benchmark::DoNotOptimize(result);
    }

    std::filesystem::remove(test_file);
}

static void BM_Traditional_Large(benchmark::State &bench_state) {
    generate_test_file(LARGE_FILE_LINES);

    for ([[maybe_unused]] auto iteration : bench_state) {
        auto result = dotenv::load_traditional(test_file, 1, false);
        benchmark::DoNotOptimize(result);
    }

    std::filesystem::remove(test_file);
}

#ifdef DOTENV_SIMD_ENABLED

// Benchmark SIMD + memory-mapping
static void BM_SIMD_MMAP_Small(benchmark::State &bench_state) {
    generate_test_file(SMALL_FILE_LINES);

    for ([[maybe_unused]] auto iteration : bench_state) {
        auto result = dotenv::simd::load_simd_mmap(test_file);
        benchmark::DoNotOptimize(result);
    }

    std::filesystem::remove(test_file);
}

static void BM_SIMD_MMAP_Large(benchmark::State &bench_state) {
    generate_test_file(LARGE_FILE_LINES);

    for ([[maybe_unused]] auto iteration : bench_state) {
        auto result = dotenv::simd::load_simd_mmap(test_file);
        benchmark::DoNotOptimize(result);
    }

    std::filesystem::remove(test_file);
}

// Benchmark pure memory-mapping vs SIMD
static void BM_MMAP_vs_SIMD_Large(benchmark::State &bench_state) {
    generate_test_file(LARGE_FILE_LINES);

    for ([[maybe_unused]] auto iteration : bench_state) {
        try {
            dotenv::mapped_file mmap_file(test_file);
            auto content = mmap_file.view();

            // Count lines with SIMD vs manual counting
            const auto simd_count =
                dotenv::simd::count_lines_avx2(content, '\n');
            benchmark::DoNotOptimize(simd_count);

        } catch (const std::exception &) {
            // Handle errors silently for benchmark
        }
    }

    std::filesystem::remove(test_file);
}

#endif // DOTENV_SIMD_ENABLED

// Register the benchmarks (simpler approach)
BENCHMARK(BM_Traditional_Small);
BENCHMARK(BM_Traditional_Large);

#ifdef DOTENV_SIMD_ENABLED
BENCHMARK(BM_SIMD_MMAP_Small);
BENCHMARK(BM_SIMD_MMAP_Large);
BENCHMARK(BM_MMAP_vs_SIMD_Large);
#endif

BENCHMARK_MAIN();
