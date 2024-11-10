#include <gtest/gtest.h>
#include <keyvaluestore/MemoryKeyValueStore.hpp>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

using namespace keyvaluestore;
using namespace std::chrono_literals;

class MemoryKeyValueStoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        store = std::make_unique<MemoryKeyValueStore>();
    }

    void TearDown() override {
        store.reset();
    }

    std::unique_ptr<MemoryKeyValueStore> store;
};

// Basic functionality tests
TEST_F(MemoryKeyValueStoreTest, BasicOperations) {
    store->set(1, "key", "value");  // Simple set operation
    EXPECT_TRUE(store->exists(1, "key")) << "Key should exist after setting";
    EXPECT_EQ(std::get<std::string>(store->get(1, "key").value()), "value");
}

// Memory-specific tests
TEST_F(MemoryKeyValueStoreTest, MemoryRelease) {
    // Fill store with data
    for (int i = 0; i < 1000; ++i) {
        store->set(1, "key" + std::to_string(i), i);
    }
    
    // Remove all and verify memory is actually released
    size_t removed = store->remove_all(1);
    EXPECT_EQ(removed, 1000) << "Should have removed 1000 items";
    
    // Verify everything is gone
    for (int i = 0; i < 1000; ++i) {
        EXPECT_FALSE(store->exists(1, "key" + std::to_string(i))) 
            << "Key should not exist after removal: key" << i;
    }
}

// Added: Test all value types
TEST_F(MemoryKeyValueStoreTest, AllValueTypes) {
    // Test string
    store->set(1, "string_key", std::string("test string"));
    ASSERT_TRUE(store->get(1, "string_key").has_value());
    EXPECT_EQ(std::get<std::string>(store->get(1, "string_key").value()), "test string");

    // Test int
    store->set(1, "int_key", 42);
    ASSERT_TRUE(store->get(1, "int_key").has_value());
    EXPECT_EQ(std::get<int>(store->get(1, "int_key").value()), 42);

    // Test double
    store->set(1, "double_key", 3.14159);
    ASSERT_TRUE(store->get(1, "double_key").has_value());
    EXPECT_DOUBLE_EQ(std::get<double>(store->get(1, "double_key").value()), 3.14159);

    // Test bool
    store->set(1, "bool_key", true);
    ASSERT_TRUE(store->get(1, "bool_key").has_value());
    EXPECT_EQ(std::get<bool>(store->get(1, "bool_key").value()), true);
}

// Added: Test value overwriting with different types
TEST_F(MemoryKeyValueStoreTest, ValueTypeOverwriting) {
    store->set(1, "key", "string value");
    EXPECT_EQ(std::get<std::string>(store->get(1, "key").value()), "string value");

    store->set(1, "key", 42);
    EXPECT_EQ(std::get<int>(store->get(1, "key").value()), 42);

    store->set(1, "key", 3.14);
    EXPECT_DOUBLE_EQ(std::get<double>(store->get(1, "key").value()), 3.14);

    store->set(1, "key", true);
    EXPECT_EQ(std::get<bool>(store->get(1, "key").value()), true);
}

// Added: Test script isolation with multiple operations
TEST_F(MemoryKeyValueStoreTest, CompleteScriptIsolation) {
    // Set up data for multiple scripts
    store->set(1, "shared_key", "script1");
    store->set(2, "shared_key", "script2");
    store->set(1, "unique_key", "unique1");
    store->set(2, "another_key", "unique2");

    // Verify isolation
    EXPECT_EQ(std::get<std::string>(store->get(1, "shared_key").value()), "script1");
    EXPECT_EQ(std::get<std::string>(store->get(2, "shared_key").value()), "script2");
    
    // Remove from one script shouldn't affect other
    store->remove(1, "shared_key");
    EXPECT_FALSE(store->exists(1, "shared_key"));
    EXPECT_TRUE(store->exists(2, "shared_key"));
    
    // remove_all shouldn't affect other scripts
    store->remove_all(1);
    EXPECT_FALSE(store->exists(1, "unique_key"));
    EXPECT_TRUE(store->exists(2, "another_key"));
}

// Added: Edge cases test
TEST_F(MemoryKeyValueStoreTest, EdgeCases) {
    // Empty string key
    store->set(1, "", "empty key");
    EXPECT_TRUE(store->exists(1, ""));
    EXPECT_EQ(std::get<std::string>(store->get(1, "").value()), "empty key");

    // Empty string value
    store->set(1, "key", std::string(""));
    EXPECT_TRUE(store->exists(1, "key"));
    EXPECT_EQ(std::get<std::string>(store->get(1, "key").value()), "");

    // Negative script ID
    store->set(-1, "key", "negative script");
    EXPECT_TRUE(store->exists(-1, "key"));
    EXPECT_EQ(std::get<std::string>(store->get(-1, "key").value()), "negative script");

    // Zero script ID
    store->set(0, "key", "zero script");
    EXPECT_TRUE(store->exists(0, "key"));
    EXPECT_EQ(std::get<std::string>(store->get(0, "key").value()), "zero script");

    // Maximum script ID
    store->set(std::numeric_limits<int>::max(), "key", "max script");
    EXPECT_TRUE(store->exists(std::numeric_limits<int>::max(), "key"));
    EXPECT_EQ(std::get<std::string>(store->get(std::numeric_limits<int>::max(), "key").value()),
              "max script");
}

// Concurrent access tests
TEST_F(MemoryKeyValueStoreTest, ConcurrentReads) {
    // Setup initial data
    store->set(1, "shared_key", 42);
    
    std::vector<std::thread> readers;
    std::atomic<int> successful_reads{0};
    
    // Spawn multiple reader threads
    for (int i = 0; i < 10; ++i) {
        readers.emplace_back([&]() {
            for (int j = 0; j < 1000; ++j) {
                auto value = store->get(1, "shared_key");
                if (value && std::get<int>(value.value()) == 42) {
                    successful_reads++;
                }
            }
        });
    }
    
    // Join all readers
    for (auto& thread : readers) {
        thread.join();
    }
    
    EXPECT_EQ(successful_reads, 10000) 
        << "All concurrent reads should have succeeded";
}

// Added: Enhanced concurrent writers test with verification
TEST_F(MemoryKeyValueStoreTest, ConcurrentWritersWithVerification) {
    std::vector<std::thread> writers;
    std::atomic<int> successful_writes{0};
    std::atomic<int> verified_values{0};
    
    // Spawn multiple writer threads
    for (int i = 0; i < 10; ++i) {
        writers.emplace_back([&, i]() {
            for (int j = 0; j < 100; ++j) {
                std::string key = "key" + std::to_string(i) + "_" + std::to_string(j);
                int value = i * 1000 + j;
                store->set(i, key, value);
                
                // Immediate verification
                auto result = store->get(i, key);
                if (result.has_value() && std::get<int>(result.value()) == value) {
                    successful_writes++;
                }
                
                // Add delay to increase contention
                if (j % 10 == 0) {
                    std::this_thread::sleep_for(1ms);
                }
            }
        });
    }
    
    // Join all writers
    for (auto& thread : writers) {
        thread.join();
    }
    
    // Final verification
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 100; ++j) {
            std::string key = "key" + std::to_string(i) + "_" + std::to_string(j);
            auto result = store->get(i, key);
            if (result.has_value() && 
                std::get<int>(result.value()) == i * 1000 + j) {
                verified_values++;
            }
        }
    }
    
    EXPECT_EQ(successful_writes, 1000) 
        << "All concurrent writes should have succeeded";
    EXPECT_EQ(verified_values, 1000)
        << "All values should be correctly stored and retrievable";
}

TEST_F(MemoryKeyValueStoreTest, ReadWriteContention) {
    std::vector<std::thread> threads;
    std::atomic<bool> stop{false};
    std::atomic<int> successful_ops{0};
    
    // Start reader threads
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&]() {
            while (!stop) {
                if (store->get(1, "contended_key").has_value()) {
                    successful_ops++;
                }
            }
        });
    }
    
    // Start writer threads
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&]() {
            int count = 0;
            while (!stop) {
                store->set(1, "contended_key", count++);
                successful_ops++;
            }
        });
    }
    
    // Let them run for a short time
    std::this_thread::sleep_for(100ms);
    stop = true;
    
    // Join all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_GT(successful_ops, 0) 
        << "Should have completed some operations under contention";
}

// Added: Test mixed operations under contention
TEST_F(MemoryKeyValueStoreTest, MixedOperationsContention) {
    std::vector<std::thread> threads;
    std::atomic<bool> stop{false};
    std::atomic<int> successful_ops{0};
    
    // Mixed operations thread function
    auto mixed_ops = [&](int thread_id) {
        while (!stop) {
            // Randomly choose operation
            int op = std::rand() % 4;
            std::string key = "key" + std::to_string(thread_id);
            
            switch (op) {
                case 0: // Set
                    store->set(thread_id, key, thread_id);
                    successful_ops++;
                    break;
                    
                case 1: // Get
                    if (store->get(thread_id, key).has_value()) {
                        successful_ops++;
                    }
                    break;
                    
                case 2: // Exists
                    if (store->exists(thread_id, key)) {
                        successful_ops++;
                    }
                    break;
                    
                case 3: // Remove
                    if (store->remove(thread_id, key)) {
                        successful_ops++;
                    }
                    break;
            }
        }
    };
    
    // Start threads performing mixed operations
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back(mixed_ops, i);
    }
    
    // Let them run for a short time
    std::this_thread::sleep_for(200ms);
    stop = true;
    
    // Join all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    EXPECT_GT(successful_ops, 0) 
        << "Should have completed some mixed operations under contention";
}

// Value size tests
TEST_F(MemoryKeyValueStoreTest, LargeValues) {
    std::string large_value(1024 * 1024, 'X'); // 1MB string
    store->set(1, "large_key", large_value);
    auto retrieved = store->get(1, "large_key");
    ASSERT_TRUE(retrieved.has_value()) << "Should be able to retrieve large value";
    EXPECT_EQ(std::get<std::string>(retrieved.value()), large_value) 
        << "Large value should match after retrieval";
}
