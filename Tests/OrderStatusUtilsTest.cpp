#include <gtest/gtest.h>
#include "../Model/Order.h"

TEST(OrderStatusUtilsTest, StatusToStr_AllValues) {
    EXPECT_EQ(orderStatusToStr(OrderStatus::RESERVED),  "RESERVED");
    EXPECT_EQ(orderStatusToStr(OrderStatus::REJECTED),  "REJECTED");
    EXPECT_EQ(orderStatusToStr(OrderStatus::PRODUCING), "PRODUCING");
    EXPECT_EQ(orderStatusToStr(OrderStatus::CONFIRMED), "CONFIRMED");
    EXPECT_EQ(orderStatusToStr(OrderStatus::RELEASED),  "RELEASED");
}

TEST(OrderStatusUtilsTest, StrToStatus_AllValues) {
    EXPECT_EQ(strToOrderStatus("RESERVED"),  OrderStatus::RESERVED);
    EXPECT_EQ(strToOrderStatus("REJECTED"),  OrderStatus::REJECTED);
    EXPECT_EQ(strToOrderStatus("PRODUCING"), OrderStatus::PRODUCING);
    EXPECT_EQ(strToOrderStatus("CONFIRMED"), OrderStatus::CONFIRMED);
    EXPECT_EQ(strToOrderStatus("RELEASED"),  OrderStatus::RELEASED);
}

TEST(OrderStatusUtilsTest, StrToStatus_Unknown_ReturnsReserved) {
    EXPECT_EQ(strToOrderStatus("UNKNOWN"), OrderStatus::RESERVED);
    EXPECT_EQ(strToOrderStatus(""),        OrderStatus::RESERVED);
}

TEST(OrderStatusUtilsTest, RoundTrip_AllValues) {
    for (auto status : { OrderStatus::RESERVED, OrderStatus::REJECTED,
                         OrderStatus::PRODUCING, OrderStatus::CONFIRMED,
                         OrderStatus::RELEASED }) {
        EXPECT_EQ(strToOrderStatus(orderStatusToStr(status)), status);
    }
}
