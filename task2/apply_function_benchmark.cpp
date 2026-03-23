#include <benchmark/benchmark.h>
#include <vector>
#include <cmath>
#include "apply_function.cpp"  


// Случай 1: однопоточная версия быстрее 
static void BM_SingleThread_LightWork(benchmark::State& state) {
    const int size = 100;
    std::vector<int> data(size, 42);
    
    std::function<void(int&)> light_func = [](int& x) { x++; };
    
    for (auto _ : state) {
        ApplyFunction(data, light_func, 1);
    }
}
BENCHMARK(BM_SingleThread_LightWork);

static void BM_MultiThread_LightWork(benchmark::State& state) {
    const int size = 100;
    std::vector<int> data(size, 42);
    
    std::function<void(int&)> light_func = [](int& x) { x++; };
    
    for (auto _ : state) {
        ApplyFunction(data, light_func, 4);
    }
}
BENCHMARK(BM_MultiThread_LightWork);


// Случай 2: многопоточная версия быстрее 
static void BM_SingleThread_HeavyWork(benchmark::State& state) {
    const int size = 10000;
    std::vector<double> data(size, 1.0);
    
    std::function<void(double&)> heavy_func = [](double& x) {
        for (int i = 0; i < 100; ++i) {
            x = std::sin(x) * std::cos(x) + std::sqrt(std::abs(x)) + std::exp(x * 0.1);
        }
    };
    
    for (auto _ : state) {
        ApplyFunction(data, heavy_func, 1);
    }
}
BENCHMARK(BM_SingleThread_HeavyWork);

static void BM_MultiThread_HeavyWork(benchmark::State& state) {
    const int size = 10000;
    std::vector<double> data(size, 1.0);
    
    std::function<void(double&)> heavy_func = [](double& x) {
        for (int i = 0; i < 100; ++i) {
            x = std::sin(x) * std::cos(x) + std::sqrt(std::abs(x)) + std::exp(x * 0.1);
        }
    };
    
    for (auto _ : state) {
        ApplyFunction(data, heavy_func, 4);
    }
}
BENCHMARK(BM_MultiThread_HeavyWork);


// Исследование влияния размера 
static void BM_SizeScaling(benchmark::State& state) {
    const int size = state.range(0);
    std::vector<double> data(size, 1.0);
    
    std::function<void(double&)> medium_func = [](double& x) {
        for (int i = 0; i < 10; ++i) {
            x = std::sin(x) * std::cos(x);
        }
    };
    
    for (auto _ : state) {
        ApplyFunction(data, medium_func, 4);
    }
}

BENCHMARK(BM_SizeScaling)
    ->RangeMultiplier(10)
    ->Range(1000, 100000);


// Сравнение разного количества потоков 
static void BM_ThreadCount(benchmark::State& state) {
    const int threads = state.range(0);
    const int size = 1000;
    std::vector<double> data(size, 1.0);
    
    std::function<void(double&)>  func = [](double& x) {
        for (int i = 0; i < 50; ++i) {
            x = std::sin(x) * std::cos(x);
        }
    };
    
    for (auto _ : state) {
        ApplyFunction(data, func, threads);
    }
}

BENCHMARK(BM_ThreadCount)
    ->Arg(1)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(5)
    ->Arg(6);

BENCHMARK_MAIN();