#pragma once
#include <string>

enum class OrderStatus {
    RESERVED,
    REJECTED,
    PRODUCING,
    CONFIRMED,
    RELEASED
};

inline std::string orderStatusToStr(OrderStatus s) {
    switch (s) {
    case OrderStatus::RESERVED:  return "RESERVED";
    case OrderStatus::REJECTED:  return "REJECTED";
    case OrderStatus::PRODUCING: return "PRODUCING";
    case OrderStatus::CONFIRMED: return "CONFIRMED";
    case OrderStatus::RELEASED:  return "RELEASED";
    default:                     return "RESERVED";
    }
}

inline OrderStatus strToOrderStatus(const std::string& s) {
    if (s == "REJECTED")  return OrderStatus::REJECTED;
    if (s == "PRODUCING") return OrderStatus::PRODUCING;
    if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
    if (s == "RELEASED")  return OrderStatus::RELEASED;
    return OrderStatus::RESERVED;
}

struct Order {
    std::string  orderId;
    std::string  sampleId;
    std::string  customer;
    int          quantity;
    OrderStatus  status;
    std::string  createdAt;  // "YYYY-MM-DD HH:MM:SS"
};
