cmake_minimum_required(VERSION 3.12)

# Benchmark executable
if(benchmark_FOUND OR TARGET benchmark::benchmark)
    add_executable(dotenv_benchmarks
        bench_main.cpp
        bench_core_operations.cpp
        bench_file_operations.cpp
        bench_thread_safety.cpp
    )

    target_link_libraries(dotenv_benchmarks
        PRIVATE
            dotenv_lib
            benchmark::benchmark
    )

    target_compile_features(dotenv_benchmarks PRIVATE cxx_std_20)

    # Performance flags para benchmarks
    target_compile_options(dotenv_benchmarks PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-O3 -march=native -flto>
        $<$<CXX_COMPILER_ID:Clang>:-O3 -march=native -flto>
        $<$<CXX_COMPILER_ID:MSVC>:/O2 /GL>
    )

    target_link_options(dotenv_benchmarks PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-flto>
        $<$<CXX_COMPILER_ID:Clang>:-flto>
        $<$<CXX_COMPILER_ID:MSVC>:/LTCG>
    )

    if (DOTENV_COMPILER_FLAGS)
        target_compile_options(dotenv_benchmarks PRIVATE ${DOTENV_COMPILER_FLAGS})
    endif()
    if (DOTENV_LINKER_FLAGS)
        target_link_options(dotenv_benchmarks PRIVATE ${DOTENV_LINKER_FLAGS})
    endif()

    # Não usar sanitizers em benchmarks (afeta performance)
    get_target_property(DOTENV_COMPILE_OPTIONS dotenv_lib COMPILE_OPTIONS)
    if(DOTENV_COMPILE_OPTIONS)
        # Remove sanitizer flags
        list(FILTER DOTENV_COMPILE_OPTIONS EXCLUDE REGEX "fsanitize")
        target_compile_options(dotenv_benchmarks PRIVATE ${DOTENV_COMPILE_OPTIONS})
    endif()

else()
    message(WARNING "Google Benchmark not available, skipping benchmarks")
endif()
