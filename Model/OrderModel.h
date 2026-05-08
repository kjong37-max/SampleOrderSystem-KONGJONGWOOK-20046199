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

    // 채번 상태 getter/setter (OrderStore용)
    std::string lastDate() const { return lastDate_; }
    int         dailySeq() const { return dailySeq_; }
    void        setLastDate(const std::string& d) { lastDate_ = d; }
    void        setDailySeq(int seq)              { dailySeq_ = seq; }

    // 파일에서 복원 시 직접 삽입 (채번 없이)
    void addDirect(const Order& order);

private:
    std::vector<Order> orders_;
    std::string        lastDate_;
    int                dailySeq_{ 0 };

    std::string generateOrderId();
};
