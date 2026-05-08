#pragma once
#include "../Model/OrderModel.h"
#include "../Model/SampleModel.h"
#include "../View/OrderView.h"

class ReleaseController {
public:
    ReleaseController(OrderModel&  orderModel,
                      SampleModel& sampleModel,
                      OrderView&   view);
    void run();

private:
    void showConfirmedList(const std::vector<Order>& orders) const;

    OrderModel&  orderModel_;
    SampleModel& sampleModel_;
    OrderView&   view_;
};
