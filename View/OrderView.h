#pragma once
#include "../Model/Order.h"
#include "../Model/Sample.h"
#include <span>
#include <string>
#include <vector>

class OrderView {
public:
    virtual ~OrderView() = default;
    virtual Order promptOrderInput(std::span<const Sample> samples) const;
    virtual void  showOrderResult(const Order& order) const;
    virtual void  showReservedList(const std::vector<Order>& orders,
                                   std::span<const Sample>   samples) const;
    virtual void  showMenu()                           const;
    virtual void  showMessage(const std::string& msg) const;
    virtual void  showError(const std::string& msg)   const;
    virtual int   promptMenuChoice()                  const;
    virtual void  showConfirmedList(const std::vector<Order>& orders) const;
};
