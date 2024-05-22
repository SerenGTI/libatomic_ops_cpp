
#include <atomic/atomic.hpp>
#include <atomic>
#include <iostream>


template<typename T1, typename T2>
auto operator<<(std::ostream& out, std::pair<T1, T2> pair) -> std::ostream&
{
    out << pair.first << ", " << pair.second;
    return out;
}

int main() {
    ao::datomic<int64_t, int64_t> l;

    l.store(5, 6, std::memory_order_release);

    auto val = l.load(std::memory_order::memory_order_acquire);

    std::cout << val << std::endl;

    auto expected1 = 5;
    auto expected2 = 6;
    std::cout << val << std::endl;

    if(l.compare_exchange_strong(expected1, expected2, 7, 8, std::memory_order_acq_rel)) {
        std::cout << "success" << std::endl;
    }
    else {
        std::cout << "failure" << std::endl;
    }

    auto old1 = 5;
    auto old2 = 6;
    l.compare_exchange_strong(old1, old2, 6, 7, std::memory_order_relaxed);

    std::cout << old1 << old2 << std::endl;

}

