#include <atomic>
#include <memory>
#include <numeric>
#include <vector>

template <typename T>
class ThreadSafeVector {
 public:
    ThreadSafeVector() : data_(std::make_shared<std::vector<T>>()) {}

    // read operation
    int Sum() const {
        auto data_copy = std::atomic_load(&data_);
        return std::accumulate(data_copy->begin(), data_copy->end(), 0);
    }

    // write operation
    void Add(int i) {
        auto data_copy = std::atomic_load(&data_);
        bool is_exchange = false;
        while (!is_exchange) {
            auto data_deep_copy = std::make_shared<std::vector<int>>(*data_copy);
            data_deep_copy->push_back(i);
            // is_exchange = std::atomic_compare_exchange_strong(&data_, &data_copy, data_deep_copy);
            is_exchange = std::atomic_compare_exchange_strong(&data_, &data_copy, std::move(data_deep_copy));
        }
    }

 private:
    std::shared_ptr<std::vector<T>> data_;
};

int main() {}