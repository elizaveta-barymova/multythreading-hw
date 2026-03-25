#include <mutex>
#include <condition_variable>
#include <optional>
#include <thread>
#include <queue>
#include <functional>
#include <iostream>
#include <exception>


template<typename T>
class SharedState {
private:
    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;
    std::optional<T> result;
    std::exception_ptr exception;

public:
    void SetValue(T value) {
        std::lock_guard lock(mtx);
        result = std::move(value);
        ready = true;
        cv.notify_all();
    }

    void SetException(std::exception_ptr e) {
        std::lock_guard lock(mtx);
        exception = e;
        ready = true;
        cv.notify_all();
    }

    T Get() {
        std::unique_lock lock(mtx);
        cv.wait(lock, [this] { return ready; });

        if (exception) {
            std::rethrow_exception(exception);
        }

        return *result;
    }
};


template<typename T>
class Future {
private:
    std::shared_ptr<SharedState<T>> state;
public:
    Future(std::shared_ptr<SharedState<T>> state)
        : state(state) {}

    T get() {
        return state->Get();
    }
};


class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop = false;

public:
    ThreadPool(size_t n) {
        for (size_t i = 0; i < n; ++i) {
            workers.emplace_back([this] {
                Worker();
            });
        }
    }

    ~ThreadPool() {
        {
            std::lock_guard lock(mtx);
            stop = true;
        }
        cv.notify_all();

        for (auto& t : workers) {
            t.join();
        }
    }
    
    template<typename F>
    auto Submit(F task) {
        using ResultType = decltype(task());

        auto state = std::make_shared<SharedState<ResultType>>();
        Future<ResultType> future(state);

        {
            std::lock_guard lock(mtx);

            tasks.emplace([task, state]() {
                try {
                    state->SetValue(task());
                } catch (...) {
                    state->SetException(std::current_exception());
                }
            });
        }

        cv.notify_one();
        return future;
}

private:
    void Worker() {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock lock(mtx);
                cv.wait(lock, [this] {
                    return stop || !tasks.empty();
                });

                if (stop && tasks.empty()) {
                    return;
                }

                task = std::move(tasks.front());
                tasks.pop();
            }

            task();
        }
    }
};


int main() {
    ThreadPool pool(4);
    
    // Передача значений
    auto f1 = pool.Submit([] { return 10; });
    auto f2 = pool.Submit([] { return 20; });
    std::cout << "Sum = " << f1.get() + f2.get() << "\n";

    // Исключения
    auto f3 = pool.Submit([]() -> int {
        throw std::runtime_error("error");
    });

    try {
        f3.get();
    } catch (const std::exception& e) {
        std::cout << "exception: " << e.what() << "\n";
    }

    // Многозадачность
    std::vector<Future<int>> futures;

    for (int i = 0; i < 100; ++i) {
        futures.push_back(pool.Submit([i] {
            return i * i;
        }));
    }

    int sum = 0;
    for (auto& f : futures) {
        sum += f.get();
    }

    std::cout << "Sum of squares = " << sum << "\n";

    return 0;
}