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
};
