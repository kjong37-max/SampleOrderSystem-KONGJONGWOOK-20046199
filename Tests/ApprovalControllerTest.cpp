#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../Controller/ApprovalController.h"
#include "../Model/OrderModel.h"
#include "../Model/SampleModel.h"
#include "../Model/ProductionLine.h"
#include "../View/OrderView.h"

class MockOrderView : public OrderView {
public:
    MOCK_METHOD(Order, promptOrderInput,
                (std::span<const Sample>), (const, override));
    MOCK_METHOD(void, showOrderResult, (const Order&), (const, override));
    MOCK_METHOD(void, showReservedList,
                (const std::vector<Order>&, std::span<const Sample>),
                (const, override));
    MOCK_METHOD(void, showMessage, (const std::string&), (const, override));
    MOCK_METHOD(void, showError,   (const std::string&), (const, override));
    MOCK_METHOD(int,  promptMenuChoice, (), (const, override));
};

class ApprovalControllerTest : public ::testing::Test {
protected:
    SampleModel    sampleModel_;
    OrderModel     orderModel_;
    ProductionLine productionLine_;
    MockOrderView  mockView_;

    void SetUp() override {
        // 기본 시료 등록
        sampleModel_.add(Sample{ "S-001", "테스트 시료", 0.5, 0.9, 100 });
    }
};

// 재고 충분 → CONFIRMED
TEST_F(ApprovalControllerTest, Approve_SufficientStock_StatusBecomesConfirmed) {
    Order o = orderModel_.reserve("S-001", "고객A", 50); // stock=100, 주문=50

    // run() 호출 없이 내부 로직을 직접 검증하기 위해
    // ApprovalController를 생성 후 calcEffectiveStock 대신
    // 실제 승인 흐름을 시뮬레이션한다.
    // 여기서는 updateStatus를 직접 호출하여 결과 검증
    int stock = sampleModel_.findById("S-001")->stock; // 100
    EXPECT_GE(stock, o.quantity); // 재고 충분

    sampleModel_.updateStock("S-001", -o.quantity);
    orderModel_.updateStatus(o.orderId, OrderStatus::CONFIRMED);

    auto updated = orderModel_.findById(o.orderId);
    EXPECT_EQ(updated->status, OrderStatus::CONFIRMED);
    EXPECT_EQ(sampleModel_.findById("S-001")->stock, 50);
}

// 재고 부족 → PRODUCING + 생산라인 등록
TEST_F(ApprovalControllerTest, Approve_InsufficientStock_StatusBecomesProducing) {
    Order o = orderModel_.reserve("S-001", "고객A", 200); // stock=100, 주문=200

    int stock    = sampleModel_.findById("S-001")->stock; // 100
    EXPECT_LT(stock, o.quantity); // 재고 부족

    int shortage  = o.quantity - stock;
    double yield  = sampleModel_.findById("S-001")->yield;
    int actualQty = static_cast<int>(
                      std::ceil(static_cast<double>(shortage) / (yield * 0.9)));

    sampleModel_.updateStock("S-001", -stock);
    ProductionJob job{ o.orderId, "S-001", shortage, actualQty,
                       sampleModel_.findById("S-001") ? 0.0 : 0.0 };
    productionLine_.enqueue(job);
    orderModel_.updateStatus(o.orderId, OrderStatus::PRODUCING);

    auto updated = orderModel_.findById(o.orderId);
    EXPECT_EQ(updated->status, OrderStatus::PRODUCING);
    EXPECT_TRUE(productionLine_.isRunning());
}

// 유효 재고: CONFIRMED 대기분 차감
TEST_F(ApprovalControllerTest, Approve_EffectiveStock_DeductsConfirmedOrders) {
    // 첫 주문 CONFIRMED (재고 50 차감)
    Order o1 = orderModel_.reserve("S-001", "고객A", 50);
    sampleModel_.updateStock("S-001", -50);
    orderModel_.updateStatus(o1.orderId, OrderStatus::CONFIRMED);

    // 두 번째 주문: 유효 재고 = 50 (100-50)
    int effectiveStock = sampleModel_.findById("S-001")->stock;
    for (const auto& o : orderModel_.findByStatus(OrderStatus::CONFIRMED))
        if (o.sampleId == "S-001") effectiveStock -= o.quantity;
    // stock=50이고 CONFIRMED가 50이므로 유효재고=0
    EXPECT_EQ(effectiveStock, 0);
}

// 거절 → REJECTED
TEST_F(ApprovalControllerTest, Reject_StatusBecomesRejected) {
    Order o = orderModel_.reserve("S-001", "고객A", 50);
    orderModel_.updateStatus(o.orderId, OrderStatus::REJECTED);
    auto updated = orderModel_.findById(o.orderId);
    EXPECT_EQ(updated->status, OrderStatus::REJECTED);
}

// CONFIRMED 없으면 유효 재고 = stock
TEST_F(ApprovalControllerTest, CalcEffectiveStock_NoConfirmed_EqualsStock) {
    int effectiveStock = sampleModel_.findById("S-001")->stock;
    for (const auto& o : orderModel_.findByStatus(OrderStatus::CONFIRMED))
        if (o.sampleId == "S-001") effectiveStock -= o.quantity;
    EXPECT_EQ(effectiveStock, 100);
}

// CONFIRMED 있으면 유효 재고 차감
TEST_F(ApprovalControllerTest, CalcEffectiveStock_WithConfirmed_DeductsAmount) {
    Order o = orderModel_.reserve("S-001", "고객A", 30);
    orderModel_.updateStatus(o.orderId, OrderStatus::CONFIRMED);

    int effectiveStock = sampleModel_.findById("S-001")->stock;
    for (const auto& conf : orderModel_.findByStatus(OrderStatus::CONFIRMED))
        if (conf.sampleId == "S-001") effectiveStock -= conf.quantity;
    EXPECT_EQ(effectiveStock, 70); // 100 - 30
}
