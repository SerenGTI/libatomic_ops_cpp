#pragma once

#include "atomic_ops.h"
#include <atomic>
#include <utility>

namespace ao {

template<typename T1, typename T2>
class datomic
{
public:
    datomic() = default;
    constexpr datomic(T1 val1, T2 val2) noexcept
    {
        value_.AO_parts.AO_v1 = val1;
        value_.AO_parts.AO_v2 = val2;
    }
    ~datomic() = default;

    datomic(const datomic&) = default;
    auto operator=(const datomic&) -> datomic& = default;
    datomic(datomic&&) noexcept = default;
    auto operator=(datomic&&) noexcept -> datomic& = default;

    auto load(std::memory_order order) const -> std::pair<T1, T2>
    {
        switch(order) {
            case std::memory_order_relaxed:
                return to_pair(AO_double_load(&value_));
            case std::memory_order_consume:
            case std::memory_order_acquire:
                return to_pair(AO_double_load_acquire(&value_));
            case std::memory_order_release:
                assert(false && "release on load?!");
            case std::memory_order_acq_rel:
            case std::memory_order_seq_cst:
                return to_pair(AO_double_load_full(&value_));
          break;
        }
        return {};
    }

    auto store(T1 val1, T2 val2, std::memory_order order) -> void
    {
        auto new_val = AO_double_t{};
        new_val.AO_parts.AO_v1 = val1;
        new_val.AO_parts.AO_v2 = val2;
        switch(order) {
            case std::memory_order_relaxed:
                AO_double_store(&value_, new_val);
            case std::memory_order_consume:
            case std::memory_order_acquire:
                assert(false && "acquire on store?!");
            case std::memory_order_release:
                AO_double_store_release(&value_, new_val);
            case std::memory_order_acq_rel:
            case std::memory_order_seq_cst:
                AO_double_store_full(&value_, new_val);
          break;
        }
    }

    template<typename TT1, typename TT2>
    auto compare_exchange_strong(TT1& old_val1,
                                 TT2& old_val2,
                                 T1 new_val1,
                                 T2 new_val2,
                                 std::memory_order success,
                                 std::memory_order failure = std::memory_order_relaxed) noexcept
        -> bool
    {
        auto res = 0;

        if(success == std::memory_order_relaxed) {
            res = AO_compare_double_and_swap_double(&value_, old_val1, old_val2, new_val1, new_val2);
        }
        else {
            res = AO_compare_double_and_swap_double_full(&value_, old_val1, old_val2, new_val1, new_val2);
        }

        if(res) {
            return true;
        }

        auto load_on_failure = load(failure);
        old_val1 = load_on_failure.first;
        old_val2 = load_on_failure.second;
        return false;
    }
private:

    constexpr static auto to_pair(AO_double_t value) noexcept -> std::pair<T1, T2>
    {
        return std::make_pair(value.AO_parts.AO_v1, value.AO_parts.AO_v2);
    }

private:
    AO_double_t value_;

};





template<typename T>
class atomic
{
public:
    atomic() = default;

    atomic(T initial_value) : value_(initial_value) {}

    ~atomic() = default;

    atomic(const atomic&) = default;
    auto operator=(const atomic&) -> atomic& = default;
    atomic(atomic&&) noexcept = default;
    auto operator=(atomic&&) noexcept -> atomic& = default;

    auto load(std::memory_order order) const -> T
    {
        switch(order) {
        case std::memory_order_relaxed: return AO_load(&value_);
        case std::memory_order_consume:
        case std::memory_order_acquire: return AO_load_acquire(&value_);
        case std::memory_order_release: assert(false && "release on load?!");
        case std::memory_order_acq_rel:
        case std::memory_order_seq_cst: return AO_load_full(&value_); break;
        }
        return {};
    }

    template<typename TT>
    auto store(TT&& val, std::memory_order order) -> void
    {
        switch(order) {
        case std::memory_order_relaxed: AO_store(&value_, std::forward<TT>(val));
        case std::memory_order_consume:
        case std::memory_order_acquire: assert(false && "acquire on store?!");
        case std::memory_order_release: AO_store_release(&value_, std::forward<TT>(val));
        case std::memory_order_acq_rel:
        case std::memory_order_seq_cst: AO_store_full(&value_, std::forward<TT>(val)); break;
        }
    }

    auto compare_exchange_strong(T& old_val,
                                 T new_val,
                                 std::memory_order success,
                                 std::memory_order failure = std::memory_order_relaxed) noexcept
        -> bool
    {
        switch(success) {
        case std::memory_order_relaxed:
            if(AO_compare_and_swap(&value_, old_val, new_val)) {
                return true;
            } else {
                old_val = load(failure);
                return false;
            }
        case std::memory_order_consume:
        case std::memory_order_acquire:
            if(AO_compare_and_swap_acquire(&value_, old_val, new_val)) {
                return true;
            } else {
                old_val = load(failure);
                return false;
            }
        case std::memory_order_release:
            if(AO_compare_and_swap_release(&value_, old_val, new_val)) {
                return true;
            } else {
                old_val = load(failure);
                return false;
            }
        case std::memory_order_acq_rel:
        case std::memory_order_seq_cst:
            if(AO_compare_and_swap_full(&value_, old_val, new_val)) {
                return true;
            } else {
                old_val = load(failure);
                return false;
            }
            break;
        }
        return false;
    }

private:
    AO_t value_ = 0;
};

} // namespace AO