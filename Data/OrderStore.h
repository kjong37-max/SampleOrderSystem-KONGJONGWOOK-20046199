#pragma once
#include "DataStore.h"
#include "../Model/OrderModel.h"

class OrderStore {
public:
    explicit OrderStore(const std::string& filePath);

    void load(OrderModel& model);
    void save(const OrderModel& model) const;

private:
    std::string filePath_;

    static std::string statusToStr(OrderStatus s);
    static OrderStatus strToStatus(const std::string& s);
};
