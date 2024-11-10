#include <gtest/gtest.h>
#include <keyvaluestore/SQLiteKeyValueStore.hpp>
#include <filesystem>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include <iostream>
#include <fstream>

using namespace keyvaluestore;
using namespace std::chrono_literals;

class SQLiteKeyValueStoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use a temporary file for testing
        temp_db_path_ = std::filesystem::temp_directory_path() / 
                       ("test_db_" + std::to_string(std::random_device{}()) + ".db");
        store = std::make_unique<SQLiteKeyValueStore>(temp_db_path_.string());
    }

    void TearDown() override {
        store.reset();  // Close the database before removal
        if (std::filesystem::exists(temp_db_path_)) {
            std::filesystem::remove(temp_db_path_);
        }
    }

    std::unique_ptr<SQLiteKeyValueStore> store;
    std::filesystem::path temp_db_path_;
};

// Basic operations tests
TEST_F(SQLiteKeyValueStoreTest, BasicOperations) {
    // Test basic set and get
    store->set(1, "test_key", "test_value");
    auto result = store->get(1, "test_key");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<std::string>(result.value()), "test_value");

    // Test exists
    EXPECT_TRUE(store->exists(1, "test_key"));
    EXPECT_FALSE(store->exists(1, "nonexistent_key"));

    // Test remove
    EXPECT_TRUE(store->remove(1, "test_key"));
    EXPECT_FALSE(store->exists(1, "test_key"));
}

// Value type tests
TEST_F(SQLiteKeyValueStoreTest, ValueTypes) {
    // String value
    store->set(1, "string_key", std::string("test string"));
    auto str_result = store->get(1, "string_key");
    ASSERT_TRUE(str_result.has_value());
    EXPECT_EQ(std::get<std::string>(str_result.value()), "test string");

    // Integer value
    store->set(1, "int_key", 42);
    auto int_result = store->get(1, "int_key");
    ASSERT_TRUE(int_result.has_value());
    EXPECT_EQ(std::get<int>(int_result.value()), 42);

    // Double value
    store->set(1, "double_key", 3.14159);
    auto double_result = store->get(1, "double_key");
    ASSERT_TRUE(double_result.has_value());
    EXPECT_DOUBLE_EQ(std::get<double>(double_result.value()), 3.14159);

    // Boolean value
    store->set(1, "bool_key", true);
    auto bool_result = store->get(1, "bool_key");
    ASSERT_TRUE(bool_result.has_value());
    EXPECT_EQ(std::get<bool>(bool_result.value()), true);
}

// Script isolation tests
TEST_F(SQLiteKeyValueStoreTest, ScriptIsolation) {
    // Set values for different scripts
    store->set(1, "shared_key", "value1");
    store->set(2, "shared_key", "value2");

    // Check isolation
    auto result1 = store->get(1, "shared_key");
    auto result2 = store->get(2, "shared_key");
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(std::get<std::string>(result1.value()), "value1");
    EXPECT_EQ(std::get<std::string>(result2.value()), "value2");

    // Test remove_all for one script
    EXPECT_EQ(store->remove_all(1), 1);
    EXPECT_FALSE(store->exists(1, "shared_key"));
    EXPECT_TRUE(store->exists(2, "shared_key"));
}

// Edge cases tests
TEST_F(SQLiteKeyValueStoreTest, EdgeCases) {
    // Empty string key
    store->set(1, "", "empty key");
    auto empty_key_result = store->get(1, "");
    ASSERT_TRUE(empty_key_result.has_value());
    EXPECT_EQ(std::get<std::string>(empty_key_result.value()), "empty key");

    // Empty string value
    store->set(1, "key", std::string(""));
    auto empty_value_result = store->get(1, "key");
    ASSERT_TRUE(empty_value_result.has_value());
    EXPECT_EQ(std::get<std::string>(empty_value_result.value()), "");

    // Special characters in key
    const std::string special_key = "!@#$%^&*()_+-=[]{}|;:'\",.<>?/\\";
    store->set(1, special_key, "special");
    auto special_result = store->get(1, special_key);
    ASSERT_TRUE(special_result.has_value());
    EXPECT_EQ(std::get<std::string>(special_result.value()), "special");

    // Very large values
    std::string large_value(1024 * 1024, 'X'); // 1MB string
    store->set(1, "large_key", large_value);
    auto large_result = store->get(1, "large_key");
    ASSERT_TRUE(large_result.has_value());
    EXPECT_EQ(std::get<std::string>(large_result.value()), large_value);
}

// Value overwrite tests
TEST_F(SQLiteKeyValueStoreTest, ValueOverwrite) {
    // Test overwriting with same type
    store->set(1, "key", "original");
    store->set(1, "key", "updated");
    auto result = store->get(1, "key");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<std::string>(result.value()), "updated");

    // Test overwriting with different types
    store->set(1, "key", 42);
    result = store->get(1, "key");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<int>(result.value()), 42);

    store->set(1, "key", true);
    result = store->get(1, "key");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<bool>(result.value()), true);
}

// Concurrent access tests
TEST_F(SQLiteKeyValueStoreTest, ConcurrentAccess) {
    const int num_threads = 10;
    const int ops_per_thread = 100;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([this, i, ops_per_thread]() {
            for (int j = 0; j < ops_per_thread; ++j) {
                std::string key = "key_" + std::to_string(i) + "_" + std::to_string(j);
                
                // Write
                store->set(i, key, j);
                
                // Read
                auto result = store->get(i, key);
                ASSERT_TRUE(result.has_value());
                EXPECT_EQ(std::get<int>(result.value()), j);
                
                // Verify exists
                EXPECT_TRUE(store->exists(i, key));
                
                // Remove
                EXPECT_TRUE(store->remove(i, key));
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

// Multiple database test
TEST_F(SQLiteKeyValueStoreTest, MultipleDatabases) {
    auto temp_db_path2 = std::filesystem::temp_directory_path() / 
                        ("test_db_2_" + std::to_string(std::random_device{}()) + ".db");
    
    auto store2 = std::make_unique<SQLiteKeyValueStore>(temp_db_path2.string());

    // Set values in both stores
    store->set(1, "key", "value1");
    store2->set(1, "key", "value2");

    // Verify isolation between databases
    auto result1 = store->get(1, "key");
    auto result2 = store2->get(1, "key");
    
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());
    EXPECT_EQ(std::get<std::string>(result1.value()), "value1");
    EXPECT_EQ(std::get<std::string>(result2.value()), "value2");

    // Cleanup
    store2.reset();
    std::filesystem::remove(temp_db_path2);
}

// Error handling tests
TEST_F(SQLiteKeyValueStoreTest, ErrorHandling) {
    // Test invalid database path
    EXPECT_THROW(SQLiteKeyValueStore("/invalid/path/to/db.sqlite"), KeyValueStoreError);

    // Test operations after closing database
    store.reset();
    store = std::make_unique<SQLiteKeyValueStore>(temp_db_path_.string());
    EXPECT_NO_THROW(store->set(1, "key", "value"));
}

// Boundary value tests
TEST_F(SQLiteKeyValueStoreTest, BoundaryValues) {
    // Test with minimum and maximum integer values
    store->set(std::numeric_limits<int>::min(), "min_script_key", "value");
    store->set(std::numeric_limits<int>::max(), "max_script_key", "value");

    EXPECT_TRUE(store->exists(std::numeric_limits<int>::min(), "min_script_key"));
    EXPECT_TRUE(store->exists(std::numeric_limits<int>::max(), "max_script_key"));

    // Test with minimum and maximum numeric values
    store->set(1, "min_int", std::numeric_limits<int>::min());
    store->set(1, "max_int", std::numeric_limits<int>::max());
    store->set(1, "min_double", std::numeric_limits<double>::min());
    store->set(1, "max_double", std::numeric_limits<double>::max());

    auto min_int_result = store->get(1, "min_int");
    auto max_int_result = store->get(1, "max_int");
    auto min_double_result = store->get(1, "min_double");
    auto max_double_result = store->get(1, "max_double");

    ASSERT_TRUE(min_int_result.has_value());
    ASSERT_TRUE(max_int_result.has_value());
    ASSERT_TRUE(min_double_result.has_value());
    ASSERT_TRUE(max_double_result.has_value());

    EXPECT_EQ(std::get<int>(min_int_result.value()), std::numeric_limits<int>::min());
    EXPECT_EQ(std::get<int>(max_int_result.value()), std::numeric_limits<int>::max());
    EXPECT_DOUBLE_EQ(std::get<double>(min_double_result.value()), std::numeric_limits<double>::min());
    EXPECT_DOUBLE_EQ(std::get<double>(max_double_result.value()), std::numeric_limits<double>::max());
}

// Performance stress test
TEST_F(SQLiteKeyValueStoreTest, StressTest) {
    const int num_operations = 1000;
    
    // Bulk write test
    auto start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_operations; ++i) {
        store->set(1, "key" + std::to_string(i), i);
    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto write_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Bulk read test
    start_time = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_operations; ++i) {
        auto result = store->get(1, "key" + std::to_string(i));
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(std::get<int>(result.value()), i);
    }
    end_time = std::chrono::high_resolution_clock::now();
    auto read_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Bulk delete test
    start_time = std::chrono::high_resolution_clock::now();
    size_t removed = store->remove_all(1);
    end_time = std::chrono::high_resolution_clock::now();
    auto delete_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    EXPECT_EQ(removed, num_operations);
    
    // Log performance metrics (optional)
    std::cout << "Performance metrics for " << num_operations << " operations:" << std::endl
              << "Write time: " << write_duration.count() << "ms" << std::endl
              << "Read time: " << read_duration.count() << "ms" << std::endl
              << "Bulk delete time: " << delete_duration.count() << "ms" << std::endl;
}

// Transactional consistency test
TEST_F(SQLiteKeyValueStoreTest, TransactionalConsistency) {
    const int num_pairs = 100;
    std::atomic<bool> error_occurred{false};
    std::vector<std::thread> threads;

    // Thread 1: Write pairs of related values
    threads.emplace_back([&]() {
        try {
            for (int i = 0; i < num_pairs; ++i) {
                store->set(1, "key_a_" + std::to_string(i), i);
                store->set(1, "key_b_" + std::to_string(i), i);
            }
        } catch (...) {
            error_occurred = true;
        }
    });

    // Thread 2: Read pairs and verify consistency
    threads.emplace_back([&]() {
        try {
            for (int i = 0; i < num_pairs * 10; ++i) {
                int idx = i % num_pairs;
                auto value_a = store->get(1, "key_a_" + std::to_string(idx));
                auto value_b = store->get(1, "key_b_" + std::to_string(idx));

                if (value_a.has_value() && value_b.has_value()) {
                    // If we have both values, they should match
                    EXPECT_EQ(std::get<int>(value_a.value()),
                            std::get<int>(value_b.value()));
                }
            }
        } catch (...) {
            error_occurred = true;
        }
    });

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_FALSE(error_occurred);
}

// Recovery after crash test
TEST_F(SQLiteKeyValueStoreTest, RecoveryAfterCrash) {
    // Write some data
    store->set(1, "persistent_key", "value");
    
    // Simulate crash by destroying the store without proper cleanup
    store.reset();

    // Create new store instance pointing to same database
    store = std::make_unique<SQLiteKeyValueStore>(temp_db_path_.string());

    // Verify data survived the "crash"
    auto result = store->get(1, "persistent_key");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<std::string>(result.value()), "value");
}

// Database size management test
TEST_F(SQLiteKeyValueStoreTest, DatabaseSizeManagement) {
    const size_t iterations = 1000;
    const size_t value_size = 1024; // 1KB
    std::string large_value(value_size, 'X');
    
    // Write phase
    for (size_t i = 0; i < iterations; ++i) {
        store->set(1, "key_" + std::to_string(i), large_value);
    }

    // Update phase - should reuse space
    for (size_t i = 0; i < iterations; ++i) {
        store->set(1, "key_" + std::to_string(i), "small_value");
    }

    // Delete phase
    size_t deleted = store->remove_all(1);
    EXPECT_EQ(deleted, iterations);
}

// Type safety tests
TEST_F(SQLiteKeyValueStoreTest, TypeSafety) {
    // Store different types
    store->set(1, "key", "string_value");
    auto string_result = store->get(1, "key");
    ASSERT_TRUE(string_result.has_value());
    EXPECT_NO_THROW(std::get<std::string>(string_result.value()));
    EXPECT_THROW(std::get<int>(string_result.value()), std::bad_variant_access);

    store->set(1, "key", 42);
    auto int_result = store->get(1, "key");
    ASSERT_TRUE(int_result.has_value());
    EXPECT_NO_THROW(std::get<int>(int_result.value()));
    EXPECT_THROW(std::get<std::string>(int_result.value()), std::bad_variant_access);

    store->set(1, "key", 3.14);
    auto double_result = store->get(1, "key");
    ASSERT_TRUE(double_result.has_value());
    EXPECT_NO_THROW(std::get<double>(double_result.value()));
    EXPECT_THROW(std::get<bool>(double_result.value()), std::bad_variant_access);

    store->set(1, "key", true);
    auto bool_result = store->get(1, "key");
    ASSERT_TRUE(bool_result.has_value());
    EXPECT_NO_THROW(std::get<bool>(bool_result.value()));
    EXPECT_THROW(std::get<double>(bool_result.value()), std::bad_variant_access);
}

// Unicode handling test
TEST_F(SQLiteKeyValueStoreTest, UnicodeHandling) {
    // Test with various Unicode strings
    const std::string unicode_key = "키🔑מפתח";
    const std::string unicode_value = "값🌟ערך";
    
    store->set(1, unicode_key, unicode_value);
    auto result = store->get(1, unicode_key);
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<std::string>(result.value()), unicode_value);

    // Test with emoji-only strings
    const std::string emoji_key = "🔑🗝️🔐";
    const std::string emoji_value = "📱💻🖥️";
    
    store->set(1, emoji_key, emoji_value);
    result = store->get(1, emoji_key);
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(std::get<std::string>(result.value()), emoji_value);
}

// Database corruption test
TEST_F(SQLiteKeyValueStoreTest, InvalidDatabaseFile) {
    // Close the database
    store.reset();
    
    // Write garbage data to create an invalid database file
    {
        std::ofstream file(temp_db_path_.string(), std::ios::binary);
        const char invalid_data[] = "This is not a valid SQLite database file";
        file.write(invalid_data, sizeof(invalid_data));
    }
    
    // Attempt to open invalid database should throw
    EXPECT_THROW(
        std::make_unique<SQLiteKeyValueStore>(temp_db_path_.string()),
        KeyValueStoreError
    );
}

// Test script isolation with large number of scripts
TEST_F(SQLiteKeyValueStoreTest, ManyScriptIsolation) {
    const int num_scripts = 1000;
    const std::string test_key = "shared_key";
    
    // Write different values for each script
    for (int i = 0; i < num_scripts; ++i) {
        store->set(i, test_key, i);
    }
    
    // Verify each script's value
    for (int i = 0; i < num_scripts; ++i) {
        auto result = store->get(i, test_key);
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(std::get<int>(result.value()), i);
    }
    
    // Remove random scripts and verify others remain unchanged
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, num_scripts - 1);
    
    for (int i = 0; i < num_scripts / 2; ++i) {
        int script_to_remove = dist(rng);
        store->remove_all(script_to_remove);
        
        // Verify a different script's value remains
        int other_script = (script_to_remove + 1) % num_scripts;
        auto result = store->get(other_script, test_key);
        if (store->exists(other_script, test_key)) {  // if we haven't removed it yet
            ASSERT_TRUE(result.has_value());
            EXPECT_EQ(std::get<int>(result.value()), other_script);
        }
    }
}

// Key uniqueness test
TEST_F(SQLiteKeyValueStoreTest, KeyUniqueness) {
    std::vector<std::string> similar_keys = {
        "key",
        "key ",  // with space
        " key",  // with leading space
        "key\t", // with tab
        "key\n", // with newline
        "key\r", // with carriage return
        "KEY",   // uppercase
        "kEy"    // mixed case
    };
    
    // Set different values for each similar key
    for (size_t i = 0; i < similar_keys.size(); ++i) {
        store->set(1, similar_keys[i], static_cast<int>(i));
    }
    
    // Verify each key has its own unique value
    for (size_t i = 0; i < similar_keys.size(); ++i) {
        auto result = store->get(1, similar_keys[i]);
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(std::get<int>(result.value()), i)
            << "Failed for key: '" << similar_keys[i] << "'";
    }
}
