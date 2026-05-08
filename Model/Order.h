#pragma once
#include <string>

enum class OrderStatus {
    RESERVED,
    REJECTED,
    PRODUCING,
    CONFIRMED,
    RELEASED
};

struct Order {
    std::string  orderId;
    std::string  sampleId;
    std::string  customer;
    int          quantity;
    OrderStatus  status;
    std::string  createdAt;  // "YYYY-MM-DD HH:MM:SS"
};
