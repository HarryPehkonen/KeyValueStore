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
# In your project's CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(myproject)

include(FetchContent)
FetchContent_Declare(
    keyvaluestore
    GIT_REPOSITORY https://github.com/yourusername/keyvaluestore.git
    GIT_TAG main  # or specific tag/commit
)
FetchContent_MakeAvailable(keyvaluestore)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE keyvaluestore::keyvaluestore)  # for both implementations
# OR
target_link_libraries(myapp PRIVATE keyvaluestore::memory)  # for memory-only
# OR
target_link_libraries(myapp PRIVATE keyvaluestore::sqlite)  # for sqlite-only
```

### Using CMake find_package

```cmake
# First install keyvaluestore on your system:
git clone https://github.com/yourusername/keyvaluestore.git
cd keyvaluestore
mkdir build && cd build
cmake ..
make
sudo make install

# Then in your project's CMakeLists.txt:
cmake_minimum_required(VERSION 3.14)
project(myproject)

find_package(keyvaluestore REQUIRED)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE keyvaluestore::keyvaluestore)  # for both implementations
```

### Using add_subdirectory

Alternatively, you can include the library directly in your project using `add_subdirectory`:

```cmake
# Project structure:
myproject/
├── CMakeLists.txt
├── main.cpp
└── extern/
    └── keyvaluestore/  # clone of your library

# In your project's CMakeLists.txt:
cmake_minimum_required(VERSION 3.14)
project(myproject)

add_subdirectory(extern/keyvaluestore)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE keyvaluestore::keyvaluestore)
```

### Example code

```cpp
#include <keyvaluestore/MemoryKeyValueStore.hpp>
#include <keyvaluestore/SQLiteKeyValueStore.hpp>
#include <iostream>
#include <variant>

void print_value(const std::optional<keyvaluestore::ValueType>& value) {
    if (!value) {
        std::cout << "null\n";
        return;
    }
    std::visit([](const auto& v) { std::cout << v << '\n'; }, *value);
}

int main() {
    try {
        // Using memory store
        keyvaluestore::MemoryKeyValueStore memory_store;
        memory_store.set(1, "test", "Hello from memory!");
        print_value(memory_store.get(1, "test"));

        // Using SQLite store
        keyvaluestore::SQLiteKeyValueStore sqlite_store("test.db");
        sqlite_store.set(1, "test", "Hello from SQLite!");
        print_value(sqlite_store.get(1, "test"));

        return 0;
    } catch (const keyvaluestore::KeyValueStoreError& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}
```

### Advanced CMake Usage

```cmake
cmake_minimum_required(VERSION 3.14)
project(myproject)

# Add cmake directory to module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Options
option(USE_MEMORY_STORE "Use memory-based key-value store" ON)
option(USE_SQLITE_STORE "Use SQLite-based key-value store" ON)

# Fetch KeyValueStore
include(FetchContent)
FetchContent_Declare(
    keyvaluestore
    GIT_REPOSITORY https://github.com/yourusername/keyvaluestore.git
    GIT_TAG main
    
    # Optional: Configure the library build
    CMAKE_ARGS
        -DKEYVALUESTORE_BUILD_MEMORY=${USE_MEMORY_STORE}
        -DKEYVALUESTORE_BUILD_SQLITE=${USE_SQLITE_STORE}
        -DKEYVALUESTORE_BUILD_TESTS=OFF
        -DKEYVALUESTORE_BUILD_EXAMPLES=OFF
)
FetchContent_MakeAvailable(keyvaluestore)

# Create main executable
add_executable(myapp main.cpp)

# Configure based on options
if(USE_MEMORY_STORE)
    target_compile_definitions(myapp PRIVATE USE_MEMORY_STORE)
endif()

if(USE_SQLITE_STORE)
    target_compile_definitions(myapp PRIVATE USE_SQLITE_STORE)
endif()

# Link against what we need
target_link_libraries(myapp 
    PRIVATE
        $<$<BOOL:${USE_MEMORY_STORE}>:keyvaluestore::memory>
        $<$<BOOL:${USE_SQLITE_STORE}>:keyvaluestore::sqlite>
)
```

### Configurable main.cpp

```cpp
#include <iostream>
#include <memory>

#ifdef USE_MEMORY_STORE
#include <keyvaluestore/MemoryKeyValueStore.hpp>
#endif

#ifdef USE_SQLITE_STORE
#include <keyvaluestore/SQLiteKeyValueStore.hpp>
#endif

// Factory function to create the appropriate store
std::unique_ptr<keyvaluestore::KeyValueStore> create_store(const std::string& type) {
    if (type == "memory") {
#ifdef USE_MEMORY_STORE
        return std::make_unique<keyvaluestore::MemoryKeyValueStore>();
#else
        throw std::runtime_error("Memory store not available in this build");
#endif
    } else if (type == "sqlite") {
#ifdef USE_SQLITE_STORE
        return std::make_unique<keyvaluestore::SQLiteKeyValueStore>("test.db");
#else
        throw std::runtime_error("SQLite store not available in this build");
#endif
    }
    throw std::runtime_error("Unknown store type");
}

int main(int argc, char* argv[]) {
    try {
        // Default to memory store if no argument provided
        std::string store_type = (argc > 1) ? argv[1] : "memory";
        
        auto store = create_store(store_type);
        store->set(1, "test", "Hello from " + store_type + "!");
        
        auto value = store->get(1, "test");
        if (value) {
            std::cout << "Value: " << std::get<std::string>(*value) << '\n';
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}```

## Build Configuration

### Build Types

CMake supports different build types:
```bash
mkdir build && cd build

# Debug build (includes debug symbols, no optimization)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build (optimized, no debug info)
cmake -DCMAKE_BUILD_TYPE=Release ..

# RelWithDebInfo (optimized, includes debug info)
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..

# Build with both implementations
cmake ..

# Or build with just memory implementation
cmake -DUSE_SQLITE_STORE=OFF ..

# Or build with just SQLite implementation
cmake -DUSE_MEMORY_STORE=OFF -DUSE_SQLITE_STORE=ON ..

make
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
