#include "dotenv.hpp"
#include <atomic>
#include <benchmark/benchmark.h>
#include <chrono>
#include <thread>
#include <vector>

// Benchmark para verificar thread safety e performance com concorrência
class ThreadSafetyBenchmark : public benchmark::Fixture {
  public:
    void SetUp([[maybe_unused]] const ::benchmark::State &state) override {
        // Setup inicial com algumas variáveis
        for (int i = 0; i < 100; ++i) {
            dotenv::set("THREAD_VAR_" + std::to_string(i),
                        "thread_value_" + std::to_string(i));
        }
    }

    void TearDown([[maybe_unused]] const ::benchmark::State &state) override {
        // Cleanup
        for (int i = 0; i < 100; ++i) {
            dotenv::unset("THREAD_VAR_" + std::to_string(i));
        }
    }
};

// Benchmark: múltiplas threads fazendo get() simultaneamente
BENCHMARK_DEFINE_F(ThreadSafetyBenchmark, ConcurrentReads)
(benchmark::State &state) {
    const auto num_threads = static_cast<int>(state.range(0));

    for (auto _ : state) {
        std::vector<std::thread> threads;
        std::atomic<int> operations_completed{0};

        auto start = std::chrono::high_resolution_clock::now();

        // Criar threads que fazem leituras
        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([&, t]() {
                for (int i = 0; i < 100; ++i) {
                    auto result =
                        dotenv::get("THREAD_VAR_" + std::to_string(i % 100));
                    benchmark::DoNotOptimize(result);
                    operations_completed.fetch_add(1);
                }
            });
        }

        // Aguardar todas as threads
        for (auto &thread : threads) {
            thread.join();
        }

        auto end = std::chrono::high_resolution_clock::now();
        [[maybe_unused]] auto elapsed =
            std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        benchmark::DoNotOptimize(operations_completed.load());
    }

    state.SetItemsProcessed(state.iterations() * num_threads * 100);
}
BENCHMARK_REGISTER_F(ThreadSafetyBenchmark, ConcurrentReads)
    ->Range(1, 8) // 1 to 8 threads
    ->Unit(benchmark::kMicrosecond);

// Benchmark: múltiplas threads fazendo set() simultaneamente
BENCHMARK_DEFINE_F(ThreadSafetyBenchmark, ConcurrentWrites)
(benchmark::State &state) {
    const auto num_threads = static_cast<int>(state.range(0));

    for (auto _ : state) {
        std::vector<std::thread> threads;
        std::atomic<int> operations_completed{0};

        // Criar threads que fazem escritas
        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([&, t]() {
                for (int i = 0; i < 50; ++i) {
                    std::string key = "WRITE_VAR_" + std::to_string(t) + "_" +
                                      std::to_string(i);
                    std::string value = "write_value_" + std::to_string(t) +
                                        "_" + std::to_string(i);
                    dotenv::set(key, value);
                    operations_completed.fetch_add(1);
                }
            });
        }

        // Aguardar todas as threads
        for (auto &thread : threads) {
            thread.join();
        }

        benchmark::DoNotOptimize(operations_completed.load());

        // Cleanup das variáveis criadas
        for (int t = 0; t < num_threads; ++t) {
            for (int i = 0; i < 50; ++i) {
                std::string key =
                    "WRITE_VAR_" + std::to_string(t) + "_" + std::to_string(i);
                dotenv::unset(key);
            }
        }
    }

    state.SetItemsProcessed(state.iterations() * num_threads * 50);
}
BENCHMARK_REGISTER_F(ThreadSafetyBenchmark, ConcurrentWrites)
    ->Range(1, 8) // 1 to 8 threads
    ->Unit(benchmark::kMicrosecond);

// Benchmark: mixed read/write workload
BENCHMARK_DEFINE_F(ThreadSafetyBenchmark, MixedReadWrite)
(benchmark::State &state) {
    const auto num_threads = static_cast<int>(state.range(0));

    for (auto _ : state) {
        std::vector<std::thread> threads;
        std::atomic<int> operations_completed{0};

        // Criar threads: metade faz reads, metade faz writes
        for (int t = 0; t < num_threads; ++t) {
            if (t % 2 == 0) {
                // Reader thread
                threads.emplace_back([&, t]() {
                    for (int i = 0; i < 100; ++i) {
                        auto result = dotenv::get("THREAD_VAR_" +
                                                  std::to_string(i % 100));
                        benchmark::DoNotOptimize(result);
                        operations_completed.fetch_add(1);
                    }
                });
            } else {
                // Writer thread
                threads.emplace_back([&, t]() {
                    for (int i = 0; i < 25; ++i) {
                        std::string key = "MIXED_VAR_" + std::to_string(t) +
                                          "_" + std::to_string(i);
                        std::string value = "mixed_value_" + std::to_string(t) +
                                            "_" + std::to_string(i);
                        dotenv::set(key, value);
                        operations_completed.fetch_add(1);
                    }
                });
            }
        }

        // Aguardar todas as threads
        for (auto &thread : threads) {
            thread.join();
        }

        benchmark::DoNotOptimize(operations_completed.load());

        // Cleanup das variáveis criadas pelos writers
        for (int t = 1; t < num_threads; t += 2) { // only writer threads
            for (int i = 0; i < 25; ++i) {
                std::string key =
                    "MIXED_VAR_" + std::to_string(t) + "_" + std::to_string(i);
                dotenv::unset(key);
            }
        }
    }

    state.SetItemsProcessed(state.iterations() *
                            ((num_threads / 2) * 100 + (num_threads / 2) * 25));
}
BENCHMARK_REGISTER_F(ThreadSafetyBenchmark, MixedReadWrite)
    ->Range(2, 8) // 2 to 8 threads (even numbers)
    ->Unit(benchmark::kMicrosecond);

// Benchmark: contention test (todas as threads acessando a mesma chave)
BENCHMARK_DEFINE_F(ThreadSafetyBenchmark, ContentionTest)
(benchmark::State &state) {
    const auto num_threads = static_cast<int>(state.range(0));
    const std::string contended_key = "CONTENDED_KEY";

    // Setup
    dotenv::set(contended_key, "initial_value");

    for (auto _ : state) {
        std::vector<std::thread> threads;
        std::atomic<int> operations_completed{0};

        // Todas as threads acessam a mesma chave
        for (int t = 0; t < num_threads; ++t) {
            threads.emplace_back([&, t]() {
                for (int i = 0; i < 100; ++i) {
                    if (i % 10 == 0) {
                        // Ocasionalmente escrever
                        dotenv::set(contended_key,
                                    "value_from_thread_" + std::to_string(t));
                    } else {
                        // Principalmente ler
                        auto result = dotenv::get(contended_key);
                        benchmark::DoNotOptimize(result);
                    }
                    operations_completed.fetch_add(1);
                }
            });
        }

        // Aguardar todas as threads
        for (auto &thread : threads) {
            thread.join();
        }

        benchmark::DoNotOptimize(operations_completed.load());
    }

    // Cleanup
    dotenv::unset(contended_key);

    state.SetItemsProcessed(state.iterations() * num_threads * 100);
}
BENCHMARK_REGISTER_F(ThreadSafetyBenchmark, ContentionTest)
    ->Range(1, 8) // 1 to 8 threads
    ->Unit(benchmark::kMicrosecond);
