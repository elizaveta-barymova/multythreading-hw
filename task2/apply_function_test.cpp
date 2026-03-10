#include <gtest/gtest.h>
#include "apply_function.cpp"

TEST(ApplyFunctionTest, SingleThread) {
    std::vector<int> data = {1, 2, 3};
    std::function<void(int&)> func = [](int& x) { x *= 2; };

    ApplyFunction(data, func, 1);

    ASSERT_EQ(data, std::vector<int>({2, 4, 6}));
}

TEST(ApplyFunctionTest, MultyThread) {
    std::vector<int> data(1000, 1);
    std::function<void(int&)> func = [](int& x) { x += 1; };

    ApplyFunction(data, func, 4);

    for (int val : data) {
        ASSERT_EQ(val, 2);
    }
}

TEST(ApplyFunctionTest, ThreadCountExceedsElements) {
    std::vector<int> data(5, 10);  
    std::function<void(int&)> func = [](int& x) { x = 0; };
    
    ApplyFunction(data, func, 100);
    
    for (int val : data) {
        ASSERT_EQ(val, 0);
    }
}

TEST(ApplyFunctionTest, EmptyVector) {
    std::vector<int> data;
    std::function<void(int&)> func = [](int& x) { x++; };
    
    ApplyFunction(data, func, 4);
    SUCCEED(); 
}

TEST(ApplyFunctionTest, InvalidThreadCount) {
    // Ноль потоков, должен выполниться 1
    std::vector<int> data(10, 5);
    std::function<void(int&)> func = [](int& x) { x = 1; };
    ApplyFunction(data, func, 0);
    for (int val : data) { ASSERT_EQ(val, 1); }
    
    // Отрицательное число потоков
    std::vector<int> data2(10, 5);
    ApplyFunction(data2, func, -5);
    for (int val : data2) { ASSERT_EQ(val, 1); }
}

TEST(ApplyFunctionTest, DifferentTypes) {
    std::vector<double> data_double = {1.5, 2.5, 3.5};
    std::function<void(double&)> func_double = [](double& x) { x *= 2.0; };
    ApplyFunction(data_double, func_double, 2);
    ASSERT_EQ(data_double, std::vector<double>({3.0, 5.0, 7.0}));
    
    std::vector<std::string> data_string = {"a", "b", "c"};
    std::function<void(std::string&)> func_string = [](std::string& x) { x += "!"; };
    ApplyFunction(data_string, func_string, 3);
    ASSERT_EQ(data_string, std::vector<std::string>({"a!", "b!", "c!"}));
}