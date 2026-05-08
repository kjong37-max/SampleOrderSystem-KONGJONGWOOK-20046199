#define NOMINMAX
#include <Windows.h>
#include "OrderModel.h"
#include <algorithm>
#include <format>
#include <ranges>

std::string OrderModel::generateOrderId() {
    SYSTEMTIME st;
    GetLocalTime(&st);

    std::string today = std::format("{:04d}{:02d}{:02d}", st.wYear, st.wMonth, st.wDay);

    if (today != lastDate_) {
        lastDate_  = today;
        dailySeq_  = 0;
    }

    ++dailySeq_;
    return std::format("ORD-{}-{:04d}", lastDate_, dailySeq_);
}

Order OrderModel::reserve(const std::string& sampleId,
                          const std::string& customer,
                          int                quantity) {
    Order o;
    o.orderId   = generateOrderId();
    o.sampleId  = sampleId;
    o.customer  = customer;
    o.quantity  = quantity;
    o.status    = OrderStatus::RESERVED;

    SYSTEMTIME st;
    GetLocalTime(&st);
    o.createdAt = std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}",
                              st.wYear, st.wMonth, st.wDay,
                              st.wHour, st.wMinute, st.wSecond);

    orders_.push_back(o);
    return o;
}

std::optional<Order> OrderModel::findById(const std::string& orderId) const {
    auto it = std::ranges::find_if(orders_,
        [&orderId](const Order& o) { return o.orderId == orderId; });
    if (it == orders_.end()) return std::nullopt;
    return *it;
}

std::vector<Order> OrderModel::findByStatus(OrderStatus status) const {
    std::vector<Order> result;
    std::ranges::copy_if(orders_, std::back_inserter(result),
        [status](const Order& o) { return o.status == status; });
    return result;
}

bool OrderModel::updateStatus(const std::string& orderId, OrderStatus newStatus) {
    auto it = std::ranges::find_if(orders_,
        [&orderId](const Order& o) { return o.orderId == orderId; });
    if (it == orders_.end()) return false;
    it->status = newStatus;
    return true;
}

std::span<const Order> OrderModel::all() const {
    return std::span<const Order>{orders_};
}

int OrderModel::totalCount() const {
    return static_cast<int>(orders_.size());
}
