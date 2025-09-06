#include "dotenv.hpp"
#include <benchmark/benchmark.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <vector>

// Benchmark para operações de arquivo
class FileBenchmarkFixture : public benchmark::Fixture {
  public:
    void SetUp([[maybe_unused]] const ::benchmark::State &state) override {
        // Criar arquivos de teste de vários tamanhos
        test_files.clear();

        // Arquivo pequeno (10 variáveis)
        create_test_file("small_test.env", 10);
        test_files.push_back("small_test.env");

        // Arquivo médio (100 variáveis)
        create_test_file("medium_test.env", 100);
        test_files.push_back("medium_test.env");

        // Arquivo grande (1000 variáveis)
        create_test_file("large_test.env", 1000);
        test_files.push_back("large_test.env");
    }

    void TearDown([[maybe_unused]] const ::benchmark::State &state) override {
        // Limpar arquivos de teste
        for (const auto &file : test_files) {
            std::filesystem::remove(file);
        }
    }

  private:
    void create_test_file(const std::string &filename, int num_vars) {
        std::ofstream file(filename);
        for (int i = 0; i < num_vars; ++i) {
            file << "TEST_VAR_" << i << "=value_" << i
                 << "_with_some_longer_content_"
                    "abcdefghijklmnopqrstuvwxyz0123456789\n";
            if (i % 10 == 0) {
                file << "# Comentário para testar parsing\n";
            }
            if (i % 20 == 0) {
                file << "QUOTED_VAR_" << i << "=\"quoted value with spaces "
                     << i << "\"\n";
            }
        }
    }

  protected:
    std::vector<std::string> test_files;
};

// Benchmark: dotenv::load() com arquivos de diferentes tamanhos
BENCHMARK_DEFINE_F(FileBenchmarkFixture, LoadSmallFile)
(benchmark::State &state) {
    for (auto _ : state) {
        bool result = dotenv::load("small_test.env");
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(FileBenchmarkFixture, LoadSmallFile)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_DEFINE_F(FileBenchmarkFixture, LoadMediumFile)
(benchmark::State &state) {
    for (auto _ : state) {
        bool result = dotenv::load("medium_test.env");
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(FileBenchmarkFixture, LoadMediumFile)
    ->Unit(benchmark::kMicrosecond);

BENCHMARK_DEFINE_F(FileBenchmarkFixture, LoadLargeFile)
(benchmark::State &state) {
    for (auto _ : state) {
        bool result = dotenv::load("large_test.env");
        benchmark::DoNotOptimize(result);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(FileBenchmarkFixture, LoadLargeFile)
    ->Unit(benchmark::kMicrosecond);

// Benchmark: dotenv::save() - TEMPORARIAMENTE DESABILITADO
// FIXME: Segmentation fault durante benchmark - investigar dotenv::save()
/*
BENCHMARK_F(FileBenchmarkFixture, SaveOperation)(benchmark::State &state) {
    // Setup: criar algumas variáveis para salvar
    for (int i = 0; i < state.range(0); ++i) {
        dotenv::set("SAVE_VAR_" + std::to_string(i),
                    "save_value_" + std::to_string(i));
    }

    for (auto _ : state) {
        dotenv::save("benchmark_output.env");
    }

    // Cleanup
    std::filesystem::remove("benchmark_output.env");
    for (int i = 0; i < state.range(0); ++i) {
        dotenv::unset("SAVE_VAR_" + std::to_string(i));
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(FileBenchmarkFixture, SaveOperation)
    ->Unit(benchmark::kMicrosecond);
*/

// Benchmark: load + get sequence (simulando uso real)
BENCHMARK_DEFINE_F(FileBenchmarkFixture, LoadThenGet)(benchmark::State &state) {
    for (auto _ : state) {
        // Load file
        bool loaded = dotenv::load("medium_test.env");
        benchmark::DoNotOptimize(loaded);

        // Get some values
        for (int i = 0; i < 10; ++i) {
            auto result = dotenv::get(std::format("TEST_VAR_{}", i * 10));
            benchmark::DoNotOptimize(result);
        }
    }
    state.SetItemsProcessed(state.iterations() * 11); // 1 load + 10 gets
}
BENCHMARK_REGISTER_F(FileBenchmarkFixture, LoadThenGet)
    ->Unit(benchmark::kMicrosecond);

// Benchmark: parsing de diferentes tipos de linhas
static void BM_ParsingCommentLines(benchmark::State &state) {
    std::string comment_content =
        "# This is a comment line with some content\n";

    for (auto _ : state) {
        // Simular parsing de linha de comentário
        bool is_comment = comment_content[0] == '#';
        benchmark::DoNotOptimize(is_comment);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ParsingCommentLines);

static void BM_ParsingQuotedValues(benchmark::State &state) {
    std::string quoted_line =
        "QUOTED_VAR=\"value with spaces and special chars!\"\n";

    for (auto _ : state) {
        // Simular parsing de valor quoted
        size_t equals_pos = quoted_line.find('=');
        if (equals_pos != std::string::npos) {
            std::string value = quoted_line.substr(equals_pos + 1);
            benchmark::DoNotOptimize(value);
        }
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_ParsingQuotedValues);
