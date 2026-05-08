#pragma once
#include "../Model/OrderModel.h"
#include "../Model/SampleModel.h"
#include "../View/OrderView.h"

class OrderController {
public:
    OrderController(OrderModel&  orderModel,
                    SampleModel& sampleModel,
                    OrderView&   view);
    void run();

private:
    void handleReserve();

    OrderModel&  orderModel_;
    SampleModel& sampleModel_;
    OrderView&   view_;
};
