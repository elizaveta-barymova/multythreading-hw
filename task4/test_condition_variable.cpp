#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <queue>
#include <condition_variable>
#include "condition_variable.h"


class ConditionVariableTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};


// Тест 1: Базовое ожидание и уведомление
TEST_F(ConditionVariableTest, BasicNotifyOne) {
    std::mutex mtx;
    condition_variable cv;
    bool ready = false;
    int data = 0;
    
    std::thread consumer([&]() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [&]() { return ready; });
        EXPECT_EQ(data, 42);
    });
    
    std::thread producer([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        {
            std::lock_guard<std::mutex> lock(mtx);
            data = 42;
            ready = true;
        }
        cv.notify_one();
    });
    
    producer.join();
    consumer.join();
}


// Тест 2: notify_all будит все потоки
TEST_F(ConditionVariableTest, NotifyAllWakesAllThreads) {
    std::mutex mtx;
    condition_variable cv;
    bool start = false;
    std::atomic<int> counter{0};
    const int THREADS = 10;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < THREADS; ++i) {
        threads.emplace_back([&]() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&]() { return start; });
            counter++;
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        start = true;
    }
    cv.notify_all();
    
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(counter, THREADS);
}


// Тест 3: notify_one будит только один поток
TEST_F(ConditionVariableTest, NotifyOneWakesSingleThread) {
    std::mutex mtx;
    condition_variable cv;
    bool ready = false;
    std::atomic<int> awakened{0};
    const int THREADS = 5;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < THREADS; ++i) {
        threads.emplace_back([&]() {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&]() { return ready; });
            awakened++;
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
    }
    
    cv.notify_one();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    EXPECT_EQ(awakened, 1);
    
    cv.notify_all();
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(awakened, THREADS);
}


// Тест 4: Производитель-потребитель с очередью
TEST_F(ConditionVariableTest, ProducerConsumer) {
    std::mutex mtx;
    condition_variable cv;
    std::queue<int> queue;
    const int ITEMS = 100;
    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};
    
    std::thread producer([&]() {
        for (int i = 0; i < ITEMS; ++i) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                queue.push(i);
                produced++;
            }
            cv.notify_one();
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    });
    
    std::thread consumer([&]() {
        while (consumed < ITEMS) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&]() { return !queue.empty(); });
            
            int value = queue.front();
            queue.pop();
            consumed++;
            EXPECT_EQ(value, consumed - 1);
        }
    });
    
    producer.join();
    consumer.join();
    
    EXPECT_EQ(produced, ITEMS);
    EXPECT_EQ(consumed, ITEMS);
}


// Тест 5: Сравнение со стандартной реализацией (функциональное)
TEST_F(ConditionVariableTest, CompareWithStdFunctional) {
    const int ITERATIONS = 1000;
    
    // Предложенная реализация
    {
        std::mutex mtx;
        condition_variable cv;
        bool ready = false;
        std::atomic<int> counter{0};
        
        std::thread producer([&]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    ready = true;
                }
                cv.notify_one();
                
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock, [&]() { return !ready; });
                }
            }
        });
        
        std::thread consumer([&]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [&]() { return ready; });
                counter++;
                ready = false;
                cv.notify_one();
            }
        });
        
        producer.join();
        consumer.join();
        
        EXPECT_EQ(counter, ITERATIONS);
    }
    
    // Стандартная реализация
    {
        std::mutex mtx;
        std::condition_variable cv;
        bool ready = false;
        std::atomic<int> counter{0};
        
        std::thread producer([&]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    ready = true;
                }
                cv.notify_one();
                
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock, [&]() { return !ready; });
                }
            }
        });
        
        std::thread consumer([&]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [&]() { return ready; });
                counter++;
                ready = false;
                cv.notify_one();
            }
        });
        
        producer.join();
        consumer.join();
        
        EXPECT_EQ(counter, ITERATIONS);
    }
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}