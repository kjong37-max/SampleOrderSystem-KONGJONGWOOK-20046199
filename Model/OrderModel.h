#pragma once
#include "Order.h"
#include <optional>
#include <span>
#include <string>
#include <vector>

class OrderModel {
public:
    Order reserve(const std::string& sampleId,
                  const std::string& customer,
                  int                quantity);

    std::optional<Order> findById(const std::string& orderId) const;
    std::vector<Order>   findByStatus(OrderStatus status) const;
    bool                 updateStatus(const std::string& orderId, OrderStatus newStatus);
    std::span<const Order> all() const;
    int totalCount() const;

private:
    std::vector<Order> orders_;
    std::string        lastDate_;
    int                dailySeq_{ 0 };

    std::string generateOrderId();
};
