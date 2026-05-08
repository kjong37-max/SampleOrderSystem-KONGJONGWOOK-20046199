#pragma once
#include "../Model/OrderModel.h"
#include "../Model/SampleModel.h"
#include "../Model/ProductionLine.h"
#include "../View/OrderView.h"
#include <cmath>

class ApprovalController {
public:
    ApprovalController(OrderModel&     orderModel,
                       SampleModel&    sampleModel,
                       ProductionLine& productionLine,
                       OrderView&      view);
    void run();

private:
    void handleApproval(const Order& order);

    OrderModel&     orderModel_;
    SampleModel&    sampleModel_;
    ProductionLine& productionLine_;
    OrderView&      view_;
};
