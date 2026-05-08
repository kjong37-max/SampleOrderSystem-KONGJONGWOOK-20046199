#include <gtest/gtest.h>
#include <cctype>
#include "../Utils/TimeUtils.h"

TEST(TimeUtilsTest, NowDateTime_FormatIsCorrect) {
    std::string dt = TimeUtils::nowDateTime();
    EXPECT_EQ(dt.size(), 19u);   // "YYYY-MM-DD HH:MM:SS"
    EXPECT_EQ(dt[4],  '-');
    EXPECT_EQ(dt[7],  '-');
    EXPECT_EQ(dt[10], ' ');
    EXPECT_EQ(dt[13], ':');
    EXPECT_EQ(dt[16], ':');
}

TEST(TimeUtilsTest, NowDate_FormatIsCorrect) {
    std::string d = TimeUtils::nowDate();
    EXPECT_EQ(d.size(), 8u);     // "YYYYMMDD"
    for (char c : d)
        EXPECT_TRUE(std::isdigit(static_cast<unsigned char>(c)));
}
