#pragma once

#include "Types.hpp"
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <utility>
#include <vector>

namespace ox {

template <typename T>
class HandlePool {
public:
    template <typename... Args>
    Handle emplace(Args&&... args) {
        std::uint16_t index;
        if (!free_.empty()) { index = free_.back(); free_.pop_back(); }
        else {
            if (slots_.size() >= std::numeric_limits<std::uint16_t>::max()) return InvalidHandle;
            index = static_cast<std::uint16_t>(slots_.size());
            slots_.push_back({});
        }
        Slot& slot = slots_[index];
        slot.value.emplace(std::forward<Args>(args)...);
        ++size_;
        return encode(index, slot.generation);
    }

    T* get(Handle handle) {
        const auto [index, generation] = decode(handle);
        if (handle == InvalidHandle || index >= slots_.size()) return nullptr;
        Slot& slot = slots_[index];
        return slot.value && slot.generation == generation ? &*slot.value : nullptr;
    }
    const T* get(Handle handle) const { return const_cast<HandlePool*>(this)->get(handle); }

    bool erase(Handle handle) {
        const auto [index, generation] = decode(handle);
        if (handle == InvalidHandle || index >= slots_.size()) return false;
        Slot& slot = slots_[index];
        if (!slot.value || slot.generation != generation) return false;
        slot.value.reset();
        slot.generation = static_cast<std::uint16_t>(slot.generation + 1u);
        if (slot.generation == 0) slot.generation = 1;
        free_.push_back(static_cast<std::uint16_t>(index));
        --size_;
        return true;
    }

    std::size_t size() const { return size_; }
    template <typename Fn> void forEach(Fn&& fn) {
        for (std::size_t i = 0; i < slots_.size(); ++i) if (slots_[i].value)
            fn(encode(static_cast<std::uint16_t>(i), slots_[i].generation), *slots_[i].value);
    }

private:
    struct Slot { std::optional<T> value; std::uint16_t generation{1}; };
    static Handle encode(std::uint16_t index, std::uint16_t generation) {
        return (static_cast<Handle>(generation) << 16u) | (static_cast<Handle>(index) + 1u);
    }
    static std::pair<std::size_t, std::uint16_t> decode(Handle handle) {
        const std::uint16_t low = static_cast<std::uint16_t>(handle & 0xffffu);
        return {low == 0 ? std::numeric_limits<std::size_t>::max() : static_cast<std::size_t>(low - 1u),
                static_cast<std::uint16_t>(handle >> 16u)};
    }
    std::vector<Slot> slots_;
    std::vector<std::uint16_t> free_;
    std::size_t size_{};
};

} // namespace ox
