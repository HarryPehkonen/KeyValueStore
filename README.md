# KeyValueStore Library

A high-performance, thread-safe key-value store library in C++ that provides both in-memory and SQLite-based persistent storage options. The library supports multiple value types and script-based isolation, making it ideal for applications requiring segregated data storage.

[![License: Unlicense](https://img.shields.io/badge/license-Unlicense-blue.svg)](http://unlicense.org/)

## Features

- 🚀 Thread-safe operations
- 💾 Multiple backend implementations:
  - In-memory store for high-performance temporary storage
  - SQLite-based store for persistent storage
- 🔐 Script-based data isolation
- 📦 Support for multiple value types:
  - String
  - Integer
  - Double
  - Boolean
- 🛠️ Modern C++ design (C++20)
- 📚 Comprehensive test coverage
- 🎯 Simple, consistent API

## Requirements

- C++20 compatible compiler
- CMake 3.14 or higher
- SQLite3 development libraries (for SQLite backend)
- Google Test (for building tests)

## Installation

### Using CMake FetchContent

The simplest way to use this library is to include it directly in your CMake project using FetchContent:

```cmake
include(FetchContent)
FetchContent_Declare(
    keyvaluestore
    GIT_REPOSITORY https://github.com/HarryPehkonen/KeyValueStore.git
    GIT_TAG main  # or specific tag/commit
)
FetchContent_MakeAvailable(keyvaluestore)

# Link against the library
target_link_libraries(your_target
    PRIVATE
        keyvaluestore::keyvaluestore
        $<$<BOOL:${KeyValueStore_HAVE_SQLITE_STORE}>:keyvaluestore::sqlite_store>
)
```

## Build Configuration

### Build Types

CMake supports different build types:
```bash
# Debug build (includes debug symbols, no optimization)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build (optimized, no debug info)
cmake -DCMAKE_BUILD_TYPE=Release ..

# RelWithDebInfo (optimized, includes debug info)
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
```

### Code Coverage

To build with code coverage support:
```bash
# Configure with coverage enabled (requires Debug build)
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_COVERAGE=ON ..
make

# Build and run the tests to generate coverage data
make coverage

# Open the coverage report
open build/coverage_report/index.html  # On macOS
xdg-open build/coverage_report/index.html  # On Linux
```

Requirements for coverage:
- Debug build (`CMAKE_BUILD_TYPE=Debug`)
- gcov and lcov installed on your system
- Tests enabled (`BUILD_TESTING=ON`, enabled by default)

Note: Coverage analysis with optimized builds (Release, RelWithDebInfo) may produce misleading results and is not recommended.

To install the required tools:
```bash
# Ubuntu/Debian
sudo apt-get install lcov

# macOS
brew install lcov

# Fedora
sudo dnf install lcov
```
### CMake Options

- `BUILD_MEMORY_STORE`: Build the in-memory implementation (ON by default)
- `BUILD_SQLITE_STORE`: Build the SQLite implementation (ON by default)
- `BUILD_TESTING`: Build the test suite (ON by default)
- `BUILD_DOCS`: Build documentation (ON by default)
- `BUILD_COVERAGE`: Build with coverage information (OFF by default)

### Building from Source

```bash
git clone https://github.com/HarryPehkonen/KeyValueStore.git
cd KeyValueStore
mkdir build && cd build
cmake ..
make
sudo make install
```

## Quick Start

### Basic Usage

```cpp
#include <keyvaluestore/MemoryKeyValueStore.hpp>
#include <iostream>

int main() {
    // Create an in-memory store
    keyvaluestore::MemoryKeyValueStore store;
    
    // Store different types of values
    store.set(1, "string_key", std::string("Hello, World!"));
    store.set(1, "int_key", 42);
    store.set(1, "double_key", 3.14159);
    store.set(1, "bool_key", true);
    
    // Retrieve values
    if (auto value = store.get(1, "string_key")) {
        std::cout << "String value: " 
                  << std::get<std::string>(value.value()) << std::endl;
    }
    
    // Check if key exists
    if (store.exists(1, "int_key")) {
        std::cout << "Integer value: " 
                  << std::get<int>(store.get(1, "int_key").value()) << std::endl;
    }
    
    // Remove a key
    store.remove(1, "bool_key");
    
    // Remove all keys for script ID 1
    size_t removed = store.remove_all(1);
    std::cout << "Removed " << removed << " keys" << std::endl;
}
```

### Using SQLite Backend

```cpp
#include <keyvaluestore/SQLiteKeyValueStore.hpp>
#include <iostream>

int main() {
    try {
        // Create a SQLite-backed store
        keyvaluestore::SQLiteKeyValueStore store("mydb.sqlite");
        
        // Store some values
        store.set(1, "user_name", std::string("Alice"));
        store.set(1, "user_age", 30);
        store.set(1, "premium_member", true);
        
        // Later, retrieve the values
        if (auto name = store.get(1, "user_name")) {
            std::cout << "User: " << std::get<std::string>(name.value()) << std::endl;
        }
        
        if (auto age = store.get(1, "user_age")) {
            std::cout << "Age: " << std::get<int>(age.value()) << std::endl;
        }
        
    } catch (const keyvaluestore::KeyValueStoreError& e) {
        std::cerr << "Storage error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### Script Isolation Example

```cpp
#include <keyvaluestore/MemoryKeyValueStore.hpp>
#include <iostream>

void processUserData(keyvaluestore::KeyValueStore& store, int user_id) {
    // Each user's data is isolated by using their ID as the script_id
    store.set(user_id, "last_login", std::string("2024-11-09"));
    store.set(user_id, "login_count", 42);
    
    // Data from one user doesn't interfere with another
    if (auto login = store.get(user_id, "last_login")) {
        std::cout << "User " << user_id << " last login: " 
                  << std::get<std::string>(login.value()) << std::endl;
    }
}

int main() {
    keyvaluestore::MemoryKeyValueStore store;
    
    // Process data for different users
    processUserData(store, 1);
    processUserData(store, 2);
    
    // Each user's data remains separate
    std::cout << "User 1 exists: " << store.exists(1, "last_login") << std::endl;
    std::cout << "User 2 exists: " << store.exists(2, "last_login") << std::endl;
}
```

### Thread-Safe Operations Example

```cpp
#include <keyvaluestore/MemoryKeyValueStore.hpp>
#include <thread>
#include <vector>
#include <iostream>

int main() {
    keyvaluestore::MemoryKeyValueStore store;
    std::vector<std::thread> threads;
    
    // Create multiple threads that write to the store
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&store, i]() {
            // Each thread uses its thread ID as script_id
            store.set(i, "thread_key", std::string("Thread ") + std::to_string(i));
            
            // Read back the value
            if (auto value = store.get(i, "thread_key")) {
                std::cout << "Thread " << i << " read: " 
                          << std::get<std::string>(value.value()) << std::endl;
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
}
```

## Using in Your CMake Project

### Method 1: FetchContent (Recommended)

```cmake
include(FetchContent)
FetchContent_Declare(
    keyvaluestore
    GIT_REPOSITORY https://github.com/HarryPehkonen/KeyValueStore.git
    GIT_TAG main  # or specific tag/commit
)
FetchContent_MakeAvailable(keyvaluestore)

add_executable(your_app main.cpp)
target_link_libraries(your_app
    PRIVATE
        keyvaluestore::keyvaluestore
        $<$<BOOL:${KeyValueStore_HAVE_SQLITE_STORE}>:keyvaluestore::sqlite_store>
)
```

### Method 2: Installing the Library

If you've installed the library system-wide:

```cmake
find_package(keyvaluestore REQUIRED)

add_executable(your_app main.cpp)
target_link_libraries(your_app
    PRIVATE
        keyvaluestore::keyvaluestore
        $<$<BOOL:${KeyValueStore_HAVE_SQLITE_STORE}>:keyvaluestore::sqlite_store>
)
```

## API Reference

The library provides a common interface through the `KeyValueStore` base class:

```cpp
class KeyValueStore {
public:
    virtual void set(int script_id, const std::string& key, 
                    const ValueType& value) = 0;
    virtual std::optional<ValueType> get(int script_id, 
                                       const std::string& key) = 0;
    virtual bool exists(int script_id, const std::string& key) = 0;
    virtual bool remove(int script_id, const std::string& key) = 0;
    virtual size_t remove_all(int script_id) = 0;
};
```

### Key Methods

- `set()`: Store a value with the given key for a specific script
- `get()`: Retrieve a value by key for a specific script
- `exists()`: Check if a key exists for a specific script
- `remove()`: Remove a key-value pair for a specific script
- `remove_all()`: Remove all key-value pairs for a specific script

## Performance Considerations

- The `MemoryKeyValueStore` implementation provides the fastest access but doesn't persist data
- The `SQLiteKeyValueStore` implementation:
  - Uses Write-Ahead Logging (WAL) for better concurrency
  - Maintains prepared statements for common operations
  - Provides ACID guarantees for data persistence
  - Uses indices for efficient script_id-based queries

## Thread Safety

Both implementations are thread-safe:
- `MemoryKeyValueStore` uses a shared mutex for read-write synchronization
- `SQLiteKeyValueStore` uses SQLite's built-in thread safety with additional mutex protection

## Error Handling

The library uses exceptions for error handling:
- All operations may throw `KeyValueStoreError` for serious errors
- Non-exceptional conditions (like key not found) use return values
- The SQLite implementation includes detailed error messages from the database

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For development, you can clone the repository using either:

- HTTPS: `git clone https://github.com/HarryPehkonen/KeyValueStore.git`
- SSH: `git clone git@github.com:HarryPehkonen/KeyValueStore.git`

## License

This project is released into the public domain - see the [LICENSE](LICENSE) file for details.
