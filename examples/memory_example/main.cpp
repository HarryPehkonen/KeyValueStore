#include <keyvaluestore/MemoryKeyValueStore.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <shared_mutex>
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

// Example of basic CRUD operations
void demonstrate_basic_operations(keyvaluestore::MemoryKeyValueStore& store) {
    std::cout << "\n=== Basic Operations ===\n";
    
    // Create/Update
    store.set(1, "string_key", "Hello, World!");
    store.set(1, "int_key", 42);
    store.set(1, "double_key", 3.14159);
    store.set(1, "bool_key", true);
    
    // Read
    std::cout << "string_key: ";
    print_value(store.get(1, "string_key"));
    std::cout << "\nint_key: ";
    print_value(store.get(1, "int_key"));
    std::cout << "\ndouble_key: ";
    print_value(store.get(1, "double_key"));
    std::cout << "\nbool_key: ";
    print_value(store.get(1, "bool_key"));
    std::cout << "\n";
    
    // Update
    store.set(1, "int_key", 100);
    std::cout << "Updated int_key: ";
    print_value(store.get(1, "int_key"));
    std::cout << "\n";
    
    // Delete
    store.remove(1, "bool_key");
    std::cout << "After removing bool_key: ";
    print_value(store.get(1, "bool_key"));
    std::cout << "\n";
}

// Example of script isolation
void demonstrate_script_isolation(keyvaluestore::MemoryKeyValueStore& store) {
    std::cout << "\n=== Script Isolation ===\n";
    
    // Set same key in different scripts
    store.set(1, "shared_key", "Script 1 Value");
    store.set(2, "shared_key", "Script 2 Value");
    store.set(3, "shared_key", "Script 3 Value");
    
    std::cout << "Script 1 value: ";
    print_value(store.get(1, "shared_key"));
    std::cout << "\nScript 2 value: ";
    print_value(store.get(2, "shared_key"));
    std::cout << "\nScript 3 value: ";
    print_value(store.get(3, "shared_key"));
    std::cout << "\n";
    
    // Remove all values for script 2
    size_t removed = store.remove_all(2);
    std::cout << "Removed " << removed << " entries from script 2\n";
    
    std::cout << "Script 1 value still exists: " << store.exists(1, "shared_key") << "\n";
    std::cout << "Script 2 value exists: " << store.exists(2, "shared_key") << "\n";
    std::cout << "Script 3 value still exists: " << store.exists(3, "shared_key") << "\n";
}

// Example of concurrent access
void demonstrate_concurrency(keyvaluestore::MemoryKeyValueStore& store) {
    std::cout << "\n=== Concurrent Access ===\n";
    
    std::vector<std::thread> threads;
    std::atomic<int> successful_ops{0};
    const int num_threads = 10;
    const int ops_per_thread = 1000;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Create multiple threads that perform operations
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&store, i, ops_per_thread, &successful_ops]() {
            for (int j = 0; j < ops_per_thread; ++j) {
                try {
                    // Write
                    store.set(i, "key", j);
                    
                    // Read
                    auto value = store.get(i, "key");
                    if (value && std::get<int>(*value) == j) {
                        successful_ops++;
                    }
                    
                    // Small delay to increase chance of contention
                    if (j % 100 == 0) {
                        std::this_thread::sleep_for(1ms);
                    }
                } catch (const keyvaluestore::KeyValueStoreError& e) {
                    std::cerr << "Error in thread " << i << ": " << e.what() << "\n";
                }
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
}

// Example of error handling
void demonstrate_error_handling(keyvaluestore::MemoryKeyValueStore& store) {
    std::cout << "\n=== Error Handling ===\n";
    
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
    } catch (const keyvaluestore::KeyValueStoreError& e) {
        std::cout << "Caught KeyValueStoreError: " << e.what() << "\n";
    }
}

int main() {
    try {
        keyvaluestore::MemoryKeyValueStore store;
        
        demonstrate_basic_operations(store);
        demonstrate_script_isolation(store);
        demonstrate_concurrency(store);
        demonstrate_error_handling(store);
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
