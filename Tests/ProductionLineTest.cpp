#include <gtest/gtest.h>
#include <cmath>
#include "../Model/ProductionLine.h"

static ProductionJob makeJob(const std::string& orderId, int shortage = 100,
                              int actualQty = 120, double totalMin = 60.0) {
    return ProductionJob{ orderId, "S-001", shortage, actualQty, totalMin };
}

TEST(ProductionLineTest, Enqueue_FirstJob_BecomesCurrentJob) {
    ProductionLine pl;
    pl.enqueue(makeJob("ORD-001"));
    ASSERT_TRUE(pl.currentJob().has_value());
    EXPECT_EQ(pl.currentJob()->orderId, "ORD-001");
}

TEST(ProductionLineTest, Enqueue_SecondJob_GoesToQueue) {
    ProductionLine pl;
    pl.enqueue(makeJob("ORD-001"));
    pl.enqueue(makeJob("ORD-002"));
    EXPECT_EQ(pl.pendingCount(), 1);
}

TEST(ProductionLineTest, PendingCount_ReflectsQueueSize) {
    ProductionLine pl;
    pl.enqueue(makeJob("ORD-001"));
    pl.enqueue(makeJob("ORD-002"));
    pl.enqueue(makeJob("ORD-003"));
    EXPECT_EQ(pl.pendingCount(), 2);
}

TEST(ProductionLineTest, IsRunning_WithCurrentJob_ReturnsTrue) {
    ProductionLine pl;
    pl.enqueue(makeJob("ORD-001"));
    EXPECT_TRUE(pl.isRunning());
}

TEST(ProductionLineTest, IsRunning_Empty_ReturnsFalse) {
    ProductionLine pl;
    EXPECT_FALSE(pl.isRunning());
}

TEST(ProductionLineTest, CompleteCurrentJob_ReturnsJob) {
    ProductionLine pl;
    pl.enqueue(makeJob("ORD-001"));
    auto done = pl.completeCurrentJob();
    ASSERT_TRUE(done.has_value());
    EXPECT_EQ(done->orderId, "ORD-001");
}

TEST(ProductionLineTest, CompleteCurrentJob_PromotesNextFromQueue) {
    ProductionLine pl;
    pl.enqueue(makeJob("ORD-001"));
    pl.enqueue(makeJob("ORD-002"));
    pl.completeCurrentJob();
    ASSERT_TRUE(pl.currentJob().has_value());
    EXPECT_EQ(pl.currentJob()->orderId, "ORD-002");
}

TEST(ProductionLineTest, CompleteCurrentJob_EmptyQueue_CurrentBecomesNullopt) {
    ProductionLine pl;
    pl.enqueue(makeJob("ORD-001"));
    pl.completeCurrentJob();
    EXPECT_FALSE(pl.currentJob().has_value());
}

TEST(ProductionLineTest, PendingJobs_ReturnsFIFOOrder) {
    ProductionLine pl;
    pl.enqueue(makeJob("ORD-001"));
    pl.enqueue(makeJob("ORD-002"));
    pl.enqueue(makeJob("ORD-003"));
    auto pending = pl.pendingJobs();
    ASSERT_EQ(pending.size(), 2u);
    EXPECT_EQ(pending[0].orderId, "ORD-002");
    EXPECT_EQ(pending[1].orderId, "ORD-003");
}

TEST(ProductionLineTest, ActualQty_CeilFormula_IsCorrect) {
    // 부족분=170, 수율=0.92 → ceil(170/(0.92*0.9)) = ceil(205.31) = 206
    double yield    = 0.92;
    int    shortage = 170;
    int    expected = 206;
    int    actual   = static_cast<int>(
                        std::ceil(static_cast<double>(shortage) / (yield * 0.9)));
    EXPECT_EQ(actual, expected);
}
