#include <gtest/gtest.h>
#include "../Model/SampleModel.h"

static Sample makeSample(const std::string& id, int stock = 100) {
    return Sample{ id, "테스트 시료", 0.5, 0.9, stock };
}

TEST(SampleModelTest, Add_NewSample_ReturnsTrue) {
    SampleModel m;
    EXPECT_TRUE(m.add(makeSample("S-001")));
}

TEST(SampleModelTest, Add_DuplicateId_ReturnsFalse) {
    SampleModel m;
    m.add(makeSample("S-001"));
    EXPECT_FALSE(m.add(makeSample("S-001")));
}

TEST(SampleModelTest, FindById_Exists_ReturnsValue) {
    SampleModel m;
    m.add(makeSample("S-001"));
    auto result = m.findById("S-001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id, "S-001");
}

TEST(SampleModelTest, FindById_NotExists_ReturnsNullopt) {
    SampleModel m;
    EXPECT_FALSE(m.findById("S-999").has_value());
}

TEST(SampleModelTest, FindByName_MatchesSubstring) {
    SampleModel m;
    m.add(Sample{ "S-001", "실리콘 웨이퍼-8인치", 0.5, 0.9, 100 });
    m.add(Sample{ "S-002", "GaN 에피택셀-4인치", 0.3, 0.8, 200 });
    auto result = m.findByName("웨이퍼");
    EXPECT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0].id, "S-001");
}

TEST(SampleModelTest, FindByName_NoMatch_ReturnsEmpty) {
    SampleModel m;
    m.add(makeSample("S-001"));
    EXPECT_TRUE(m.findByName("없는시료").empty());
}

TEST(SampleModelTest, UpdateStock_Positive_IncreasesStock) {
    SampleModel m;
    m.add(makeSample("S-001", 100));
    EXPECT_TRUE(m.updateStock("S-001", 50));
    EXPECT_EQ(m.findById("S-001")->stock, 150);
}

TEST(SampleModelTest, UpdateStock_NegativeBelowZero_ReturnsFalse) {
    SampleModel m;
    m.add(makeSample("S-001", 10));
    EXPECT_FALSE(m.updateStock("S-001", -20));
    EXPECT_EQ(m.findById("S-001")->stock, 10);
}

TEST(SampleModelTest, TotalStock_SumsAllSamples) {
    SampleModel m;
    m.add(makeSample("S-001", 100));
    m.add(makeSample("S-002", 200));
    EXPECT_EQ(m.totalStock(), 300);
}

TEST(SampleModelTest, Count_ReturnsCorrectCount) {
    SampleModel m;
    m.add(makeSample("S-001"));
    m.add(makeSample("S-002"));
    EXPECT_EQ(m.count(), 2);
}
