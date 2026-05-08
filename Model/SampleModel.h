#pragma once
#include "Sample.h"
#include <optional>
#include <span>
#include <string>
#include <vector>

class OrderModel;

class SampleModel {
public:
    bool add(const Sample& sample);
    std::optional<Sample> findById(const std::string& id) const;
    std::vector<Sample> findByName(const std::string& keyword) const;
    std::span<const Sample> all() const;
    bool updateStock(const std::string& id, int delta);
    int totalStock() const;
    int count() const;

    // 유효 재고 = stock - Sigma(CONFIRMED 주문 수량)
    int effectiveStock(const std::string& sampleId,
                       const OrderModel&  orderModel) const;

    // 재고 상태 판정 (고갈/부족/여유)
    StockStatus stockStatus(const std::string& sampleId,
                            const OrderModel&  orderModel) const;

private:
    std::vector<Sample> samples_;
};
