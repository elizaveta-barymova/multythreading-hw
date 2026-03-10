Решение состоит из файлов:

* apply_function.h - реализация функции
* apply_function_test.cpp - тесты
* apply_function_benchmark.cpp - бенчмарки для сравнения производительности
* CMakeLists.txt - файл для сборки конфигурации

Сборка и запуск следующими командами:

```bash
mkdir build && cd build
cmake ..
make
```

Запуск тестов:

```bash
./apply_function_tests
```

Запуск бенчмарков:

```bash
./apply_function_benchmark
```

Выводы фактически полученные на системе, на которой разрабатывалось решение:

```
Running main() from /home/kwiaty/multythreading-hw/task2/build/_deps/googletest-src/googletest/src/gtest_main.cc
[==========] Running 6 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 6 tests from ApplyFunctionTest
[ RUN      ] ApplyFunctionTest.SingleThread
[       OK ] ApplyFunctionTest.SingleThread (0 ms)
[ RUN      ] ApplyFunctionTest.MultyThread
[       OK ] ApplyFunctionTest.MultyThread (0 ms)
[ RUN      ] ApplyFunctionTest.ThreadCountExceedsElements
[       OK ] ApplyFunctionTest.ThreadCountExceedsElements (0 ms)
[ RUN      ] ApplyFunctionTest.EmptyVector
[       OK ] ApplyFunctionTest.EmptyVector (0 ms)
[ RUN      ] ApplyFunctionTest.InvalidThreadCount
[       OK ] ApplyFunctionTest.InvalidThreadCount (0 ms)
[ RUN      ] ApplyFunctionTest.DifferentTypes
[       OK ] ApplyFunctionTest.DifferentTypes (0 ms)
[----------] 6 tests from ApplyFunctionTest (1 ms total)

[----------] Global test environment tear-down
[==========] 6 tests from 1 test suite ran. (1 ms total)
[  PASSED  ] 6 tests.
```

```
Run on (3 X 2496 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x3)
  L1 Instruction 32 KiB (x3)
  L2 Unified 1280 KiB (x3)
  L3 Unified 18432 KiB (x3)
Load Average: 1.11, 0.82, 0.59
***WARNING*** Library was built as DEBUG. Timings may be affected.
--------------------------------------------------------------------
Benchmark                          Time             CPU   Iterations
--------------------------------------------------------------------
BM_SingleThread_LightWork     172798 ns        61369 ns        10000
BM_MultiThread_LightWork      229025 ns       119477 ns         6160
BM_SingleThread_HeavyWork   20892688 ns       151126 ns         1000
BM_MultiThread_HeavyWork    10328534 ns       226136 ns         1000
BM_SizeScaling/1000           227382 ns       106621 ns         6627
BM_SizeScaling/10000          845347 ns       141378 ns         4771
BM_SizeScaling/100000        6559512 ns       216795 ns         1000
BM_ThreadCount/1              928602 ns        77257 ns         8952
BM_ThreadCount/2              553475 ns        92014 ns         7353
BM_ThreadCount/3              451309 ns       135049 ns         5176
BM_ThreadCount/4              532309 ns       117872 ns         5992
BM_ThreadCount/5              614271 ns       195567 ns         3568
BM_ThreadCount/6              669658 ns       272750 ns         2587
```
