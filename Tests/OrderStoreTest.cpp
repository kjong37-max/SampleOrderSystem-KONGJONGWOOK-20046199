#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "../Data/OrderStore.h"
#include "../Model/OrderModel.h"

class OrderStoreTest : public ::testing::Test {
protected:
    const std::string testFile_ = "test_orderstore_tmp.txt";

    void TearDown() override {
        std::filesystem::remove(testFile_);
    }
};

TEST_F(OrderStoreTest, Save_And_Load_Count_Matches) {
    OrderModel m1;
    m1.reserve("S-001", "고객A", 100);
    m1.reserve("S-002", "고객B", 50);
    OrderStore(testFile_).save(m1);

    OrderModel m2;
    OrderStore(testFile_).load(m2);
    EXPECT_EQ(m2.totalCount(), 2);
}

TEST_F(OrderStoreTest, Save_And_Load_AllFields_Match) {
    OrderModel m1;
    Order o = m1.reserve("S-001", "삼성전자", 200);
    OrderStore(testFile_).save(m1);

    OrderModel m2;
    OrderStore(testFile_).load(m2);
    auto r = m2.findById(o.orderId);
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(r->sampleId,  "S-001");
    EXPECT_EQ(r->customer,  "삼성전자");
    EXPECT_EQ(r->quantity,  200);
    EXPECT_EQ(r->createdAt, o.createdAt);
}

TEST_F(OrderStoreTest, Save_And_Load_Status_Reserved) {
    OrderModel m1;
    Order o = m1.reserve("S-001", "고객A", 10);
    // 기본 상태가 RESERVED
    OrderStore(testFile_).save(m1);
    OrderModel m2; OrderStore(testFile_).load(m2);
    EXPECT_EQ(m2.findById(o.orderId)->status, OrderStatus::RESERVED);
}

TEST_F(OrderStoreTest, Save_And_Load_Status_Confirmed) {
    OrderModel m1;
    Order o = m1.reserve("S-001", "고객A", 10);
    m1.updateStatus(o.orderId, OrderStatus::CONFIRMED);
    OrderStore(testFile_).save(m1);
    OrderModel m2; OrderStore(testFile_).load(m2);
    EXPECT_EQ(m2.findById(o.orderId)->status, OrderStatus::CONFIRMED);
}

TEST_F(OrderStoreTest, Save_And_Load_Status_Producing) {
    OrderModel m1;
    Order o = m1.reserve("S-001", "고객A", 10);
    m1.updateStatus(o.orderId, OrderStatus::PRODUCING);
    OrderStore(testFile_).save(m1);
    OrderModel m2; OrderStore(testFile_).load(m2);
    EXPECT_EQ(m2.findById(o.orderId)->status, OrderStatus::PRODUCING);
}

TEST_F(OrderStoreTest, Save_And_Load_Status_Released) {
    OrderModel m1;
    Order o = m1.reserve("S-001", "고객A", 10);
    m1.updateStatus(o.orderId, OrderStatus::RELEASED);
    OrderStore(testFile_).save(m1);
    OrderModel m2; OrderStore(testFile_).load(m2);
    EXPECT_EQ(m2.findById(o.orderId)->status, OrderStatus::RELEASED);
}

TEST_F(OrderStoreTest, Save_And_Load_Status_Rejected) {
    OrderModel m1;
    Order o = m1.reserve("S-001", "고객A", 10);
    m1.updateStatus(o.orderId, OrderStatus::REJECTED);
    OrderStore(testFile_).save(m1);
    OrderModel m2; OrderStore(testFile_).load(m2);
    EXPECT_EQ(m2.findById(o.orderId)->status, OrderStatus::REJECTED);
}

TEST_F(OrderStoreTest, Save_DailySeq_RestoredCorrectly) {
    std::string secondId;
    {
        OrderModel m;
        m.reserve("S-001", "고객A", 10);
        Order o2 = m.reserve("S-001", "고객B", 20);  // dailySeq = 2
        secondId = o2.orderId;
        OrderStore(testFile_).save(m);
    }
    OrderModel m2;
    OrderStore(testFile_).load(m2);

    // 복원 후 추가 주문은 3번부터
    Order next = m2.reserve("S-001", "고객C", 30);
    EXPECT_EQ(next.orderId.substr(13), "0003");
}

TEST_F(OrderStoreTest, Load_NoFile_EmptyModel) {
    OrderModel m;
    EXPECT_NO_THROW(OrderStore("nofile_xyz_999.txt").load(m));
    EXPECT_EQ(m.totalCount(), 0);
}
