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
    OrderModel&  orderModel_;
    SampleModel& sampleModel_;
    OrderView&   view_;
};
