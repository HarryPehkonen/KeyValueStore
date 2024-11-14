#include <keyvaluestore/SQLiteKeyValueStore.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <random>
using namespace std::chrono_literals;

// Helper function to print a value of any supported type
void print_value(const std::optional<keyvaluestore::ValueType>& value) {
    if (!value) {
        std::cout << "null";
        return;
    }
    
    std::visit([](const auto& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::string>) {
            std::cout << std::quoted(v);
        } else if constexpr (std::is_same_v<T, bool>) {
            std::cout << (v ? "true" : "false");
        } else {
            std::cout << v;
        }
    }, *value);
}

// Example of basic CRUD operations with persistence
void demonstrate_persistence(const std::string& db_path) {
    std::cout << "\n=== Persistence Demo ===\n";
    
    // First store instance - write data
    {
        keyvaluestore::SQLiteKeyValueStore store(db_path);
        store.set(1, "string_key", "Hello, World!");
        store.set(1, "int_key", 42);
        store.set(1, "double_key", 3.14159);
        store.set(1, "bool_key", true);
        
        std::cout << "Data written to database.\n";
    } // store is destroyed, connection closed
    
    // Second store instance - read data back
    {
        keyvaluestore::SQLiteKeyValueStore store(db_path);
        std::cout << "Reading back from database:\n";
        std::cout << "string_key: ";
        print_value(store.get(1, "string_key"));
        std::cout << "\nint_key: ";
        print_value(store.get(1, "int_key"));
        std::cout << "\ndouble_key: ";
        print_value(store.get(1, "double_key"));
        std::cout << "\nbool_key: ";
        print_value(store.get(1, "bool_key"));
        std::cout << "\n";
    }
}

// Example of script isolation with transactions
void demonstrate_transactional_operations(const std::string& db_path) {
    std::cout << "\n=== Transactional Operations ===\n";
    keyvaluestore::SQLiteKeyValueStore store(db_path);
    
    try {
        // Update multiple values - all succeed or all fail
        store.set(1, "balance1", 1000);
        store.set(1, "balance2", 2000);
        
        std::cout << "Initial balances:\n";
        std::cout << "Balance 1: ";
        print_value(store.get(1, "balance1"));
        std::cout << "\nBalance 2: ";
        print_value(store.get(1, "balance2"));
        std::cout << "\n";
        
        // Simulate a transfer
        int amount = 500;
        store.set(1, "balance1", std::get<int>(store.get(1, "balance1").value()) - amount);
        store.set(1, "balance2", std::get<int>(store.get(1, "balance2").value()) + amount);
        
        std::cout << "After transfer of " << amount << ":\n";
        std::cout << "Balance 1: ";
        print_value(store.get(1, "balance1"));
        std::cout << "\nBalance 2: ";
        print_value(store.get(1, "balance2"));
        std::cout << "\n";
        
    } catch (const keyvaluestore::KeyValueStoreError& e) {
        std::cout << "Transaction failed: " << e.what() << "\n";
    }
}

// Example of concurrent access to SQLite store
void demonstrate_concurrent_access(const std::string& db_path) {
    std::cout << "\n=== Concurrent Access ===\n";
    
    keyvaluestore::SQLiteKeyValueStore store(db_path);
    std::vector<std::thread> threads;
    std::atomic<int> successful_ops{0};
    const int num_threads = 10;
    const int ops_per_thread = 100;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Create multiple threads that perform operations
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([db_path, i, ops_per_thread, &successful_ops]() {
            try {
                // Each thread gets its own connection
                keyvaluestore::SQLiteKeyValueStore thread_store(db_path);
                
                for (int j = 0; j < ops_per_thread; ++j) {
                    // Write
                    thread_store.set(i, "key", j);
                    
                    // Read
                    auto value = thread_store.get(i, "key");
                    if (value && std::get<int>(*value) == j) {
                        successful_ops++;
                    }
                    
                    // Small delay to increase chance of contention
                    if (j % 10 == 0) {
                        std::this_thread::sleep_for(1ms);
                    }
                }
            } catch (const keyvaluestore::KeyValueStoreError& e) {
                std::cerr << "Error in thread " << i << ": " << e.what() << "\n";
            }
        });
    }
    
    // Join all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    std::cout << "Completed " << successful_ops << " successful operations in "
              << duration.count() << "ms\n";
    std::cout << "Operations per second: "
              << (successful_ops * 1000.0 / duration.count()) << "\n";
    
    // Check database size
    std::filesystem::path db_file(db_path);
    std::cout << "Database file size: "
              << std::filesystem::file_size(db_file) << " bytes\n";
}

// Example of error handling and edge cases
void demonstrate_error_handling(const std::string& db_path) {
    std::cout << "\n=== Error Handling ===\n";
    
    try {
        // Try to open database in a non-existent directory
        keyvaluestore::SQLiteKeyValueStore invalid_store("/nonexistent/path/db.sqlite");
    } catch (const keyvaluestore::KeyValueStoreError& e) {
        std::cout << "Expected error opening invalid path: " << e.what() << "\n";
    }
    
    keyvaluestore::SQLiteKeyValueStore store(db_path);
    
    try {
        // Store a string
        store.set(1, "key", "string value");
        
        // Try to get it as an integer (this will throw)
        auto value = store.get(1, "key");
        if (value) {
            int i = std::get<int>(*value);
            std::cout << "Integer value: " << i << "\n";
        }
    } catch (const std::bad_variant_access& e) {
        std::cout << "Caught type mismatch error: " << e.what() << "\n";
    }
    
    // Test with very large values
    try {
        std::string large_value(1024 * 1024, 'X'); // 1MB string
        store.set(1, "large_key", large_value);
        auto retrieved = store.get(1, "large_key");
        if (retrieved) {
            std::cout << "Successfully stored and retrieved 1MB value\n";
        }
    } catch (const keyvaluestore::KeyValueStoreError& e) {
        std::cout << "Error with large value: " << e.what() << "\n";
    }
}

// Example of data cleanup and maintenance
void demonstrate_maintenance(const std::string& db_path) {
    std::cout << "\n=== Database Maintenance ===\n";
    
    keyvaluestore::SQLiteKeyValueStore store(db_path);
    
    // Insert some test data
    for (int i = 0; i < 100; ++i) {
        store.set(1, "temp_key_" + std::to_string(i), i);
    }
    
    // Get initial file size
    std::filesystem::path db_file(db_path);
    auto initial_size = std::filesystem::file_size(db_file);
    std::cout << "Initial database size: " << initial_size << " bytes\n";
    
    // Remove all data
    size_t removed = store.remove_all(1);
    std::cout << "Removed " << removed << " entries\n";
    
    // Get final file size
    auto final_size = std::filesystem::file_size(db_file);
    std::cout << "Final database size: " << final_size << " bytes\n";
}

int main() {
    try {
        // Create a unique database file in the system's temp directory
        std::string db_path = (std::filesystem::temp_directory_path() / 
            ("test_db_" + std::to_string(std::random_device{}()) + ".db")).string();
        
        std::cout << "Using database: " << db_path << "\n";
        
        demonstrate_persistence(db_path);
        demonstrate_transactional_operations(db_path);
        demonstrate_concurrent_access(db_path);
        demonstrate_error_handling(db_path);
        demonstrate_maintenance(db_path);
        
        // Cleanup
        std::filesystem::remove(db_path);
        std::cout << "\nDatabase file removed.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
