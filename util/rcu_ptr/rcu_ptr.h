#pragma once

#include <atomic>
#include <memory>   // std::shared_ptr
#include <utility>  // std::move

namespace util {

template <typename T>
class rcu_ptr {
 public:
    rcu_ptr() = default;
    ~rcu_ptr() = default;
    rcu_ptr(const rcu_ptr& rhs) = delete;
    rcu_ptr& operator=(const rcu_ptr& rhs) = delete;
    rcu_ptr(rcu_ptr&&) = delete;
    explicit rcu_ptr(const std::shared_ptr<const T>& sp) : sp_(sp) {}

 public:
    /**
     * @brief return a std::shared_ptr<const T> by value, therefore it is thread safe
     * 
     * @return std::shared_ptr<const T> 
     */
    inline std::shared_ptr<const T> Load() const {
        return std::atomic_load_explicit(&sp_, std::memory_order_consume);
    }

    /**
     * @brief receive a std::shared_ptr<const T> as an lvalue parameter
     *        we can use it to reset the wrapped data to a new value independent from the old value
     * 
     * @param r 
     */
    inline void Store(const std::shared_ptr<const T>& r) {
        std::atomic_store_explicit(&sp_, r, std::memory_order_release);
    }
    /**
     * @brief receive a std::shared_ptr<const T> as an rvalue reference parameter
     *        we can use it to reset the wrapped data to a new value independent from the old value
     * 
     * @param r 
     */
    inline void Store(const std::shared_ptr<const T>&& r) {
        std::atomic_store_explicit(&sp_, std::move(r), std::memory_order_release);
    }

    /**
     * @brief receive a lambda that is called whenever an update needs to be done
     *        it will be called continuously until the update is successfully
     * 
     * @tparam F is a lambda expression that receives a T* for the copy of the actual data
     * @param func
     */
    template <typename F>
    void Update(F&& func) {
        std::shared_ptr<const T> sp_copy = std::atomic_load_explicit(&sp_, std::memory_order_consume);
        std::shared_ptr<T> sp_deep_copy;
        do {
            if (sp_copy) {
                sp_deep_copy = std::make_shared<T>(*sp_copy);
            }
            std::forward<F>(func)(sp_deep_copy.get());
        } while (!std::atomic_compare_exchange_strong_explicit(&sp_, &sp_copy,
                                                               std::shared_ptr<const T>(std::move(sp_deep_copy)),
                                                               std::memory_order_release, std::memory_order_consume));
    }

 private:
    std::shared_ptr<const T> sp_;
};

}  // namespace util