#include <gtest/gtest.h>
#include <keyvaluestore/KeyValueStore.hpp>
#include <memory>

// Mock implementation for testing interface contracts
class MockKeyValueStore : public keyvaluestore::KeyValueStore {
public:
    void set(int script_id, const std::string& key, 
            const keyvaluestore::ValueType& value) override {
        last_script_id = script_id;
        store[std::make_pair(script_id, key)] = value;
    }

    std::optional<keyvaluestore::ValueType> get(int script_id, 
            const std::string& key) override {
        auto it = store.find(std::make_pair(script_id, key));
        if (it != store.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    bool exists(int script_id, const std::string& key) override {
        return store.find(std::make_pair(script_id, key)) != store.end();
    }

    bool remove(int script_id, const std::string& key) override {
        return store.erase(std::make_pair(script_id, key)) > 0;
    }

    size_t remove_all(int script_id) override {
        size_t count = 0;
        auto it = store.begin();
        while (it != store.end()) {
            if (it->first.first == script_id) {
                it = store.erase(it);
                count++;
            } else {
                ++it;
            }
        }
        return count;
    }

    // Test helpers
    int last_script_id = -1;
    std::map<std::pair<int, std::string>, keyvaluestore::ValueType> store;
};

class KeyValueStoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        store = std::make_unique<MockKeyValueStore>();
    }

    void TearDown() override {
        store.reset();
    }

    std::unique_ptr<MockKeyValueStore> store;
};

TEST_F(KeyValueStoreTest, ScriptIdIsolation) {
    store->set(1, "key", "value1");
    store->set(2, "key", "value2");

    EXPECT_EQ(std::get<std::string>(store->get(1, "key").value()), "value1");
    EXPECT_EQ(std::get<std::string>(store->get(2, "key").value()), "value2");
}

TEST_F(KeyValueStoreTest, ValueTypeSafety) {
    store->set(1, "string", std::string("test"));
    store->set(1, "int", 42);
    store->set(1, "double", 3.14);
    store->set(1, "bool", true);

    EXPECT_EQ(std::get<std::string>(store->get(1, "string").value()), "test");
    EXPECT_EQ(std::get<int>(store->get(1, "int").value()), 42);
    EXPECT_DOUBLE_EQ(std::get<double>(store->get(1, "double").value()), 3.14);
    EXPECT_EQ(std::get<bool>(store->get(1, "bool").value()), true);
}

TEST_F(KeyValueStoreTest, NonExistentKeyBehavior) {
    EXPECT_FALSE(store->get(1, "nonexistent").has_value());
    EXPECT_FALSE(store->exists(1, "nonexistent"));
    EXPECT_FALSE(store->remove(1, "nonexistent"));
}

TEST_F(KeyValueStoreTest, ExistsConsistency) {
    store->set(1, "key", "value");
    EXPECT_TRUE(store->exists(1, "key"));
    EXPECT_TRUE(store->get(1, "key").has_value());
    
    store->remove(1, "key");
    EXPECT_FALSE(store->exists(1, "key"));
    EXPECT_FALSE(store->get(1, "key").has_value());
}

TEST_F(KeyValueStoreTest, RemoveOperations) {
    store->set(1, "key1", "value1");
    store->set(1, "key2", "value2");
    store->set(2, "key1", "value3");

    EXPECT_TRUE(store->remove(1, "key1"));
    EXPECT_FALSE(store->exists(1, "key1"));
    EXPECT_TRUE(store->exists(1, "key2"));
    EXPECT_TRUE(store->exists(2, "key1"));

    size_t removed = store->remove_all(1);
    EXPECT_EQ(removed, 1);
    EXPECT_FALSE(store->exists(1, "key2"));
    EXPECT_TRUE(store->exists(2, "key1"));
}

TEST_F(KeyValueStoreTest, ValueOverwriteBehavior) {
    store->set(1, "key", "original");
    EXPECT_EQ(std::get<std::string>(store->get(1, "key").value()), "original");

    store->set(1, "key", "updated");
    EXPECT_EQ(std::get<std::string>(store->get(1, "key").value()), "updated");
}

TEST_F(KeyValueStoreTest, TypeMismatchHandling) {
    store->set(1, "key", 42);
    auto value = store->get(1, "key");
    EXPECT_TRUE(value.has_value());
    EXPECT_THROW(std::get<std::string>(value.value()), std::bad_variant_access);
}

TEST_F(KeyValueStoreTest, EmptyKeyBehavior) {
    store->set(1, "", "empty key");
    EXPECT_TRUE(store->exists(1, ""));
    EXPECT_EQ(std::get<std::string>(store->get(1, "").value()), "empty key");
}

TEST_F(KeyValueStoreTest, LargeScriptIdValues) {
    int large_id = std::numeric_limits<int>::max();
    store->set(large_id, "key", "value");
    EXPECT_TRUE(store->exists(large_id, "key"));
    EXPECT_EQ(std::get<std::string>(store->get(large_id, "key").value()), "value");
}
