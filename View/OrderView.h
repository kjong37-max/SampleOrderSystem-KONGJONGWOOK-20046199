#pragma once
#include "../Model/Order.h"
#include "../Model/Sample.h"
#include <span>
#include <string>
#include <vector>

class OrderView {
public:
    Order promptOrderInput(std::span<const Sample> samples) const;
    void showOrderResult(const Order& order) const;
    void showReservedList(const std::vector<Order>& orders,
                          std::span<const Sample>   samples) const;
    void showMessage(const std::string& msg) const;
    void showError(const std::string& msg)   const;
    int  promptMenuChoice()                  const;
};
