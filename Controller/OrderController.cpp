#include "OrderController.h"

OrderController::OrderController(OrderModel&  orderModel,
                                 SampleModel& sampleModel,
                                 OrderView&   view)
    : orderModel_(orderModel)
    , sampleModel_(sampleModel)
    , view_(view)
{
}

void OrderController::run() {
    view_.showMenu();
    handleReserve();
}

void OrderController::handleReserve() {
    Order o = view_.promptOrderInput(sampleModel_.all());
    Order created = orderModel_.reserve(o.sampleId, o.customer, o.quantity);
    view_.showOrderResult(created);
}
