#pragma once

#include <functional>
#include <vector>
#include <algorithm>

namespace foundation {

// ============================================================
// Signal<Args...> — 轻量信号/槽，不依赖 Qt
//
// 用法:
//   Signal<int, std::string> mySignal;
//   auto id = mySignal.connect([](int x, std::string s){ ... });
//   mySignal.emit(42, "hello");
//   mySignal.disconnect(id);
// ============================================================

template<typename... Args>
class Signal {
public:
    using SlotId = int;
    using Slot   = std::function<void(Args...)>;

    SlotId connect(Slot slot) {
        int id = nextId_++;
        slots_.push_back({id, std::move(slot)});
        return id;
    }

    void disconnect(SlotId id) {
        slots_.erase(
            std::remove_if(slots_.begin(), slots_.end(),
                [id](const Entry& e){ return e.id == id; }),
            slots_.end());
    }

    void disconnectAll() { slots_.clear(); }

    void emit(Args... args) const {
        // 拷贝一份避免回调中 connect/disconnect 导致迭代器失效
        auto copy = slots_;
        for (auto& entry : copy) {
            entry.slot(args...);
        }
    }

    std::size_t connectionCount() const { return slots_.size(); }

private:
    struct Entry { SlotId id; Slot slot; };
    std::vector<Entry> slots_;
    SlotId nextId_ = 0;
};

// 常用特化：属性变更信号（无参数）
using ChangeSignal = Signal<>;

} // namespace foundation
