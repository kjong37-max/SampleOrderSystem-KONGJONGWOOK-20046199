#include <gtest/gtest.h>
#include "../Model/OrderModel.h"

TEST(OrderModelTest, Reserve_CreatesReservedOrder) {
    OrderModel m;
    Order o = m.reserve("S-001", "고객A", 100);
    EXPECT_EQ(o.status, OrderStatus::RESERVED);
    EXPECT_EQ(o.sampleId, "S-001");
    EXPECT_EQ(o.quantity, 100);
}

TEST(OrderModelTest, Reserve_OrderIdFormat_IsCorrect) {
    OrderModel m;
    Order o = m.reserve("S-001", "고객A", 100);
    // "ORD-" 로 시작하는지 확인
    EXPECT_EQ(o.orderId.substr(0, 4), "ORD-");
    EXPECT_EQ(o.orderId.size(), 18u); // "ORD-YYYYMMDD-NNNN"
}

TEST(OrderModelTest, Reserve_DailySeqIncrement) {
    OrderModel m;
    Order o1 = m.reserve("S-001", "고객A", 100);
    Order o2 = m.reserve("S-001", "고객B", 50);
    // 두 번째 주문의 일련번호가 더 큰지 확인
    std::string seq1 = o1.orderId.substr(13); // "NNNN"
    std::string seq2 = o2.orderId.substr(13);
    EXPECT_LT(seq1, seq2);
}

TEST(OrderModelTest, FindById_Exists_ReturnsOrder) {
    OrderModel m;
    Order o = m.reserve("S-001", "고객A", 100);
    auto result = m.findById(o.orderId);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->orderId, o.orderId);
}

TEST(OrderModelTest, FindById_NotExists_ReturnsNullopt) {
    OrderModel m;
    EXPECT_FALSE(m.findById("ORD-99999999-9999").has_value());
}

TEST(OrderModelTest, FindByStatus_ReturnsMatchingOrders) {
    OrderModel m;
    m.reserve("S-001", "고객A", 100);
    m.reserve("S-002", "고객B", 50);
    auto reserved = m.findByStatus(OrderStatus::RESERVED);
    EXPECT_EQ(reserved.size(), 2u);
}

TEST(OrderModelTest, UpdateStatus_ChangesStatus) {
    OrderModel m;
    Order o = m.reserve("S-001", "고객A", 100);
    EXPECT_TRUE(m.updateStatus(o.orderId, OrderStatus::CONFIRMED));
    auto updated = m.findById(o.orderId);
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->status, OrderStatus::CONFIRMED);
}

TEST(OrderModelTest, UpdateStatus_NotFound_ReturnsFalse) {
    OrderModel m;
    EXPECT_FALSE(m.updateStatus("ORD-99999999-9999", OrderStatus::CONFIRMED));
}

TEST(OrderModelTest, TotalCount_ReturnsCorrectCount) {
    OrderModel m;
    m.reserve("S-001", "고객A", 100);
    m.reserve("S-002", "고객B", 50);
    EXPECT_EQ(m.totalCount(), 2);
}

TEST(OrderModelTest, AddDirect_RestoresOrder) {
    OrderModel m;
    Order o;
    o.orderId   = "ORD-20260416-0001";
    o.sampleId  = "S-001";
    o.customer  = "테스트";
    o.quantity  = 100;
    o.status    = OrderStatus::CONFIRMED;
    o.createdAt = "2026-04-16 09:00:00";
    m.addDirect(o);
    auto result = m.findById("ORD-20260416-0001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, OrderStatus::CONFIRMED);
}
