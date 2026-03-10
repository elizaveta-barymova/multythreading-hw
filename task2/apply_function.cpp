#include <thread>
#include <vector>
#include <functional>

template <typename T>
void ApplyFunction(
    std::vector<T>& data, 
    const std::function<void(T&)>& transform, 
    const int threadCount = 1
) {
    // Корректировка числа потоков
    size_t size = data.size();
    if (size == 0)
        return;

    int actualThreadCount = std::max(1, threadCount); 
    if (static_cast<size_t>(actualThreadCount) > size) {
        actualThreadCount = static_cast<int>(size);
    }

    // Создание потоков
    std::vector<std::jthread> threads;
    threads.reserve(actualThreadCount);

    // Распределение вычислений по потокам
    size_t chunckSize = size / actualThreadCount;
    size_t remainder  = size % actualThreadCount;

    size_t start_ind = 0;
    for (int i = 0; i < actualThreadCount; ++i) {
        size_t rem = (i < static_cast<int>(remainder) ? 1 : 0);
        size_t end_ind = start_ind + chunckSize + rem;

        threads.emplace_back([&data, &transform, start_ind, end_ind]() {
            for (size_t j = start_ind; j < end_ind; j++) {
                transform(data[j]);
            }
        });

        start_ind = end_ind;
    }                  
}
