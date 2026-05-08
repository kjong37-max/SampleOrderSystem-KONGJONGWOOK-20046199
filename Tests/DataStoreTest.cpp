#include <gtest/gtest.h>
#include <filesystem>
#include "../Data/DataStore.h"

class DataStoreTest : public ::testing::Test {
protected:
    const std::string testFile_ = "test_datastore_tmp.txt";

    void TearDown() override {
        std::filesystem::remove(testFile_);
    }
};

TEST_F(DataStoreTest, SetAndGet_StoresValue) {
    DataStore store(testFile_);
    store.set("key1", "hello");
    EXPECT_EQ(store.get("key1"), "hello");
}

TEST_F(DataStoreTest, Get_MissingKey_ReturnsDefault) {
    DataStore store(testFile_);
    EXPECT_EQ(store.get("missing", "default"), "default");
}

TEST_F(DataStoreTest, Has_ExistingKey_ReturnsTrue) {
    DataStore store(testFile_);
    store.set("k", "v");
    EXPECT_TRUE(store.has("k"));
}

TEST_F(DataStoreTest, Has_MissingKey_ReturnsFalse) {
    DataStore store(testFile_);
    EXPECT_FALSE(store.has("missing"));
}

TEST_F(DataStoreTest, Remove_DeletesKey) {
    DataStore store(testFile_);
    store.set("k", "v");
    store.remove("k");
    EXPECT_FALSE(store.has("k"));
}

TEST_F(DataStoreTest, Keys_ReturnsAllKeys) {
    DataStore store(testFile_);
    store.set("a", "1");
    store.set("b", "2");
    auto keys = store.keys();
    EXPECT_EQ(keys.size(), 2u);
}

TEST_F(DataStoreTest, SaveAndLoad_TXT_RoundTrip) {
    {
        DataStore store(testFile_);
        store.set("key1", "value1");
        store.set("key2", "value2");
        store.save();
    }
    DataStore store2(testFile_);
    store2.load();
    EXPECT_EQ(store2.get("key1"), "value1");
    EXPECT_EQ(store2.get("key2"), "value2");
}

TEST_F(DataStoreTest, Load_NonExistentFile_EmptyStore) {
    DataStore store("nonexistent_xyz_12345.txt");
    EXPECT_NO_THROW(store.load());
    EXPECT_TRUE(store.keys().empty());
}

TEST_F(DataStoreTest, SaveAndLoad_OverwritesExisting) {
    {
        DataStore store(testFile_);
        store.set("k", "old");
        store.save();
    }
    {
        DataStore store(testFile_);
        store.load();
        store.set("k", "new");
        store.save();
    }
    DataStore store3(testFile_);
    store3.load();
    EXPECT_EQ(store3.get("k"), "new");
}
