#include <gtest/gtest.h>
#include "../Model/SampleModel.h"
#include "../Model/OrderModel.h"
#include "../Model/ProductionLine.h"

class ProductionControllerTest : public ::testing::Test {
protected:
    SampleModel    sampleModel_;
    OrderModel     orderModel_;
    ProductionLine productionLine_;

    void SetUp() override {
        sampleModel_.add(Sample{ "S-001", "테스트 시료", 0.5, 0.9, 0 });
    }
};

TEST_F(ProductionControllerTest, Complete_NoCurrentJob_NothingChanges) {
    EXPECT_FALSE(productionLine_.isRunning());
    int stockBefore = sampleModel_.findById("S-001")->stock;
    // 처리 중 작업 없음 → 아무것도 변경되지 않음
    EXPECT_EQ(sampleModel_.findById("S-001")->stock, stockBefore);
    EXPECT_EQ(orderModel_.totalCount(), 0);
}

TEST_F(ProductionControllerTest, Complete_Job_StockIncreasesByActualQty) {
    Order o = orderModel_.reserve("S-001", "고객A", 50);
    orderModel_.updateStatus(o.orderId, OrderStatus::PRODUCING);
    ProductionJob job{ o.orderId, "S-001", 50, 62, 31.0 };
    productionLine_.enqueue(job);

    auto current = productionLine_.currentJob();
    ASSERT_TRUE(current.has_value());
    sampleModel_.updateStock(current->sampleId, current->actualQty);
    orderModel_.updateStatus(current->orderId, OrderStatus::CONFIRMED);
    productionLine_.completeCurrentJob();

    EXPECT_EQ(sampleModel_.findById("S-001")->stock, 62);
}

TEST_F(ProductionControllerTest, Complete_Job_StatusBecomesConfirmed) {
    Order o = orderModel_.reserve("S-001", "고객A", 50);
    orderModel_.updateStatus(o.orderId, OrderStatus::PRODUCING);
    ProductionJob job{ o.orderId, "S-001", 50, 62, 31.0 };
    productionLine_.enqueue(job);

    auto current = productionLine_.currentJob();
    ASSERT_TRUE(current.has_value());
    sampleModel_.updateStock(current->sampleId, current->actualQty);
    orderModel_.updateStatus(current->orderId, OrderStatus::CONFIRMED);
    productionLine_.completeCurrentJob();

    EXPECT_EQ(orderModel_.findById(o.orderId)->status, OrderStatus::CONFIRMED);
}

TEST_F(ProductionControllerTest, Complete_Job_ProductionLineAdvances) {
    Order o1 = orderModel_.reserve("S-001", "고객A", 50);
    Order o2 = orderModel_.reserve("S-001", "고객B", 30);
    orderModel_.updateStatus(o1.orderId, OrderStatus::PRODUCING);
    orderModel_.updateStatus(o2.orderId, OrderStatus::PRODUCING);

    productionLine_.enqueue({ o1.orderId, "S-001", 50, 62, 31.0 });
    productionLine_.enqueue({ o2.orderId, "S-001", 30, 37, 18.5 });

    auto current = productionLine_.currentJob();
    ASSERT_TRUE(current.has_value());
    sampleModel_.updateStock(current->sampleId, current->actualQty);
    orderModel_.updateStatus(current->orderId, OrderStatus::CONFIRMED);
    productionLine_.completeCurrentJob();

    // 다음 작업이 current로 승격
    ASSERT_TRUE(productionLine_.currentJob().has_value());
    EXPECT_EQ(productionLine_.currentJob()->orderId, o2.orderId);
}

TEST_F(ProductionControllerTest, Complete_LastJob_ProductionLineBecomesIdle) {
    Order o = orderModel_.reserve("S-001", "고객A", 50);
    orderModel_.updateStatus(o.orderId, OrderStatus::PRODUCING);
    productionLine_.enqueue({ o.orderId, "S-001", 50, 62, 31.0 });

    auto current = productionLine_.currentJob();
    ASSERT_TRUE(current.has_value());
    sampleModel_.updateStock(current->sampleId, current->actualQty);
    orderModel_.updateStatus(current->orderId, OrderStatus::CONFIRMED);
    productionLine_.completeCurrentJob();

    EXPECT_FALSE(productionLine_.isRunning());
}
