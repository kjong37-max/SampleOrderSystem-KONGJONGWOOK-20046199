#include <gtest/gtest.h>
#include "../Model/OrderModel.h"
#include "../Model/SampleModel.h"

class ReleaseControllerTest : public ::testing::Test {
protected:
    SampleModel sampleModel_;
    OrderModel  orderModel_;

    void SetUp() override {
        sampleModel_.add(Sample{ "S-001", "테스트 시료", 0.5, 0.9, 100 });
    }
};

TEST_F(ReleaseControllerTest, Release_ChangesStatusToReleased) {
    Order o = orderModel_.reserve("S-001", "고객A", 30);
    orderModel_.updateStatus(o.orderId, OrderStatus::CONFIRMED);
    orderModel_.updateStatus(o.orderId, OrderStatus::RELEASED);
    EXPECT_EQ(orderModel_.findById(o.orderId)->status, OrderStatus::RELEASED);
}

TEST_F(ReleaseControllerTest, Release_DoesNotChangeStock) {
    sampleModel_.updateStock("S-001", -30);  // stock = 70
    Order o = orderModel_.reserve("S-001", "고객A", 30);
    orderModel_.updateStatus(o.orderId, OrderStatus::CONFIRMED);

    int stockBefore = sampleModel_.findById("S-001")->stock;
    orderModel_.updateStatus(o.orderId, OrderStatus::RELEASED);

    EXPECT_EQ(sampleModel_.findById("S-001")->stock, stockBefore);
}

TEST_F(ReleaseControllerTest, Release_OnlyConfirmedOrders_AreReleasable) {
    Order o1 = orderModel_.reserve("S-001", "고객A", 10);
    Order o2 = orderModel_.reserve("S-001", "고객B", 20);
    orderModel_.updateStatus(o1.orderId, OrderStatus::CONFIRMED);
    // o2는 RESERVED 유지

    auto confirmed = orderModel_.findByStatus(OrderStatus::CONFIRMED);
    ASSERT_EQ(confirmed.size(), 1u);
    EXPECT_EQ(confirmed[0].orderId, o1.orderId);
}

TEST_F(ReleaseControllerTest, Release_MultipleConfirmed_EachReleasableIndependently) {
    Order o1 = orderModel_.reserve("S-001", "고객A", 10);
    Order o2 = orderModel_.reserve("S-001", "고객B", 20);
    orderModel_.updateStatus(o1.orderId, OrderStatus::CONFIRMED);
    orderModel_.updateStatus(o2.orderId, OrderStatus::CONFIRMED);

    // o1만 출고
    orderModel_.updateStatus(o1.orderId, OrderStatus::RELEASED);

    auto confirmed = orderModel_.findByStatus(OrderStatus::CONFIRMED);
    EXPECT_EQ(confirmed.size(), 1u);
    EXPECT_EQ(confirmed[0].orderId, o2.orderId);
}
