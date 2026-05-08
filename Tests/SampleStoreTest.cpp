#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include "../Data/SampleStore.h"
#include "../Model/SampleModel.h"

class SampleStoreTest : public ::testing::Test {
protected:
    const std::string testFile_ = "test_samplestore_tmp.txt";

    void TearDown() override {
        std::filesystem::remove(testFile_);
    }

    void populateModel(SampleModel& model) {
        model.add({ "S-001", "실리콘 웨이퍼-8인치", 0.5, 0.92, 480 });
        model.add({ "S-002", "GaN 에피택셀-4인치",  0.3, 0.78, 220 });
        model.add({ "S-003", "SiC 파워기판-6인치",  0.8, 0.92,  30 });
        model.add({ "S-004", "포토레지스트-PR7",    0.2, 0.95, 910 });
        model.add({ "S-005", "산화막 웨이퍼-SiO2", 0.6, 0.88,   0 });
    }
};

TEST_F(SampleStoreTest, Save_And_Load_Count_Matches) {
    SampleModel m1;
    populateModel(m1);
    SampleStore store(testFile_);
    store.save(m1);

    SampleModel m2;
    SampleStore store2(testFile_);
    store2.load(m2);
    EXPECT_EQ(m2.count(), 5);
}

TEST_F(SampleStoreTest, Save_And_Load_Id_Matches) {
    SampleModel m1;
    populateModel(m1);
    SampleStore(testFile_).save(m1);

    SampleModel m2;
    SampleStore(testFile_).load(m2);
    ASSERT_TRUE(m2.findById("S-001").has_value());
    ASSERT_TRUE(m2.findById("S-005").has_value());
}

TEST_F(SampleStoreTest, Save_And_Load_AllFields_Match) {
    SampleModel m1;
    m1.add({ "S-001", "실리콘 웨이퍼-8인치", 0.5, 0.92, 480 });
    SampleStore(testFile_).save(m1);

    SampleModel m2;
    SampleStore(testFile_).load(m2);
    auto s = m2.findById("S-001");
    ASSERT_TRUE(s.has_value());
    EXPECT_EQ(s->name, "실리콘 웨이퍼-8인치");
    EXPECT_DOUBLE_EQ(s->avgProdTime, 0.5);
    EXPECT_DOUBLE_EQ(s->yield, 0.92);
    EXPECT_EQ(s->stock, 480);
}

TEST_F(SampleStoreTest, Save_UpdatedStock_Persists) {
    SampleModel m1;
    m1.add({ "S-001", "테스트", 0.5, 0.9, 100 });
    m1.updateStock("S-001", -30);  // stock = 70
    SampleStore(testFile_).save(m1);

    SampleModel m2;
    SampleStore(testFile_).load(m2);
    EXPECT_EQ(m2.findById("S-001")->stock, 70);
}

TEST_F(SampleStoreTest, Load_EmptyFile_EmptyModel) {
    // 빈 파일 생성
    { std::ofstream f(testFile_); }
    SampleModel m;
    SampleStore(testFile_).load(m);
    EXPECT_EQ(m.count(), 0);
}

TEST_F(SampleStoreTest, Load_NoFile_EmptyModel) {
    SampleModel m;
    EXPECT_NO_THROW(SampleStore("nofile_xyz_999.txt").load(m));
    EXPECT_EQ(m.count(), 0);
}
