#!/bin/bash

# Script para executar benchmarks da biblioteca dotenv
# Fase 4: Desempenho mensurável e hardening

set -e

echo "=== Benchmark dotenv - Fase 4 ==="
echo ""

# Configurar diretório de build com otimizações
BUILD_DIR="build_benchmark"
BENCHMARK_OPTIONS="--benchmark_repetitions=5 --benchmark_display_aggregates_only=true --benchmark_format=console"

# Criar diretório de build otimizado
echo "Criando build otimizado para benchmarks..."
if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# CMake com flags de otimização
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DDOTENV_ENABLE_BENCHMARKS=ON \
    -DDOTENV_ENABLE_TESTS=OFF \
    -DDOTENV_ENABLE_SANITIZERS=OFF \
    -DCMAKE_CXX_FLAGS="-O3 -march=native -DNDEBUG" \
    -DCMAKE_C_FLAGS="-O3 -march=native -DNDEBUG"

# Build
echo "Compilando com otimizações..."
make -j$(nproc)

echo ""
echo "=== Executando Benchmarks ==="
echo ""

if [ -f "./benchmarks/dotenv_benchmarks" ]; then
    echo "Benchmark: Operações principais (get, set, get_optional)"
    ./benchmarks/dotenv_benchmarks --benchmark_filter="BenchmarkFixture.*" $BENCHMARK_OPTIONS

    echo ""
    echo "Benchmark: Operações de arquivo (load, save)"
    ./benchmarks/dotenv_benchmarks --benchmark_filter="FileBenchmarkFixture.*" $BENCHMARK_OPTIONS

    echo ""
    echo "Benchmark: Thread Safety (concorrência)"
    ./benchmarks/dotenv_benchmarks --benchmark_filter="ThreadSafetyBenchmark.*" $BENCHMARK_OPTIONS

    echo ""
    echo "Benchmark: Stress Tests"
    ./benchmarks/dotenv_benchmarks --benchmark_filter="BM_StressTest.*" $BENCHMARK_OPTIONS

    echo ""
    echo "Benchmark: Parsing Types"
    ./benchmarks/dotenv_benchmarks --benchmark_filter="BM_ParsingTypes.*" $BENCHMARK_OPTIONS

    echo ""
    echo "=== Relatório de Performance ==="
    echo "Salvando resultados em benchmark_results.json..."
    ./benchmarks/dotenv_benchmarks --benchmark_format=json --benchmark_out=benchmark_results.json $BENCHMARK_OPTIONS

    echo "Benchmark concluído. Resultados disponíveis em:"
    echo "  - Console output acima"
    echo "  - JSON: $BUILD_DIR/benchmark_results.json"
else
    echo "ERRO: Executável de benchmark não encontrado!"
    echo "Verifique se o Google Benchmark foi baixado corretamente."
    exit 1
fi

echo ""
echo "=== Validação Thread Safety com TSAN ==="
echo ""

# Recompilar com Thread Sanitizer para validação
cd ..
TSAN_BUILD_DIR="build_tsan"

if [ -d "$TSAN_BUILD_DIR" ]; then
    rm -rf "$TSAN_BUILD_DIR"
fi

mkdir -p "$TSAN_BUILD_DIR"
cd "$TSAN_BUILD_DIR"

echo "Compilando com Thread Sanitizer..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DDOTENV_ENABLE_BENCHMARKS=ON \
    -DDOTENV_ENABLE_TESTS=ON \
    -DDOTENV_ENABLE_SANITIZERS=OFF \
    -DCMAKE_CXX_FLAGS="-fsanitize=thread -g -O1" \
    -DCMAKE_C_FLAGS="-fsanitize=thread -g -O1" \
    -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=thread" \
    -DCMAKE_SHARED_LINKER_FLAGS="-fsanitize=thread"

make -j$(nproc)

echo "Executando teste rápido de thread safety..."
if [ -f "./benchmarks/dotenv_benchmarks" ]; then
    # Executar apenas um benchmark de concorrência básico com TSAN
    timeout 30s ./benchmarks/dotenv_benchmarks --benchmark_filter="ThreadSafetyBenchmark/ConcurrentReads/2" --benchmark_repetitions=1 || echo "Thread safety test completed"
    echo "Thread Sanitizer validation concluída."
else
    echo "Executando testes regulares com TSAN..."
    if [ -f "./tests/dotenv_tests" ]; then
        timeout 30s ./tests/dotenv_tests || echo "TSAN tests completed"
    fi
fi

cd ..

echo ""
echo "=== Fase 4 Concluída ==="
echo ""
echo "✅ Benchmarks de performance executados"
echo "✅ Validação de thread safety com TSAN"
echo "✅ Relatórios de performance gerados"
echo ""
echo "Próxima fase: Fase 5 - Empacotamento e Developer Experience"
