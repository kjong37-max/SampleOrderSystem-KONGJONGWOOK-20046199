#pragma once
#include "../Model/OrderModel.h"
#include "../Model/SampleModel.h"
#include "../Model/ProductionLine.h"
#include "../View/ProductionView.h"

class ProductionController {
public:
    ProductionController(OrderModel&     orderModel,
                         SampleModel&    sampleModel,
                         ProductionLine& productionLine,
                         ProductionView& view);
    void run();

private:
    void handleStatus();
    void handleComplete();

    OrderModel&     orderModel_;
    SampleModel&    sampleModel_;
    ProductionLine& productionLine_;
    ProductionView& view_;
};
