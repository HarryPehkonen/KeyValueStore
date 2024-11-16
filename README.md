# KeyValueStore Library

A high-performance, thread-safe key-value store library in C++ that provides both in-memory and optional SQLite-based persistent storage. The library supports multiple value types and script-based isolation, making it ideal for applications requiring segregated data storage.

## Features

* Thread-safe operations
* Multiple backend implementations:
  * In-memory store for high-performance temporary storage
  * Optional SQLite-based store for persistent storage
* Script-based data isolation
* Support for multiple value types:
  * String
  * Integer
  * Double
  * Boolean
* Modern C++ design (C++20)
* Comprehensive test coverage
* Simple, consistent API

## Requirements

* C++20 compatible compiler
* CMake 3.14 or higher
* SQLite3 development libraries (only if SQLite backend is enabled)
* Google Test (for building tests)

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
    GIT_REPOSITORY https://github.com/HarryPehkonen/KeyValueStore.git
    GIT_TAG main  # or specific tag/commit
)
FetchContent_MakeAvailable(keyvaluestore)

# Basic usage (memory-only)
add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE keyvaluestore::keyvaluestore)

# Optional: Enable SQLite support during fetch
FetchContent_Declare(
    keyvaluestore
    GIT_REPOSITORY https://github.com/HarryPehkonen/KeyValueStore.git
    GIT_TAG main
    CMAKE_ARGS
        -DKEYVALUESTORE_USE_SQLITE=ON
)
```

### Using CMake find_package

```cmake
# First install keyvaluestore on your system:
git clone https://github.com/HarryPehkonen/KeyValueStore.git
cd keyvaluestore
mkdir build && cd build

# Basic build (memory-only)
cmake ..
# OR with SQLite support
cmake -DKEYVALUESTORE_USE_SQLITE=ON ..

make
sudo make install

# Then in your project's CMakeLists.txt, if you did the "sudo make
# install" part:
cmake_minimum_required(VERSION 3.14)
project(myproject)

find_package(keyvaluestore REQUIRED)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE keyvaluestore::keyvaluestore)
```

### Using add_subdirectory

You can include the library directly in your project using `add_subdirectory`:

```cmake
# Project structure:
myproject/
├── CMakeLists.txt
├── main.cpp
└── extern/
    └── keyvaluestore/  # clone of this library

# In your project's CMakeLists.txt:
cmake_minimum_required(VERSION 3.14)
project(myproject)

# Optional: Enable SQLite support
set(KEYVALUESTORE_USE_SQLITE ON CACHE BOOL "Enable SQLite backend")

add_subdirectory(extern/keyvaluestore)

add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE keyvaluestore::keyvaluestore)
```

## Usage Examples

### Basic Usage with Memory Store

```cpp
#include <keyvaluestore/KeyValueStore.hpp>
#include <iostream>

int main() {
    try {
        // Create an in-memory store
        auto store = keyvaluestore::KeyValueStore::createInMemory();
        
        // Store different types of values
        store->set(1, "string_key", std::string("Hello, World!"));
        store->set(1, "int_key", 42);
        store->set(1, "double_key", 3.14159);
        store->set(1, "bool_key", true);
        
        // Retrieve values
        if (auto value = store->get(1, "string_key")) {
            std::cout << "String value: " 
                     << std::get<std::string>(value.value()) << std::endl;
        }
        
        // Check if key exists
        if (store->exists(1, "int_key")) {
            std::cout << "Integer value: " 
                     << std::get<int>(store->get(1, "int_key").value()) << std::endl;
        }
        
        // Remove a key
        store->remove(1, "bool_key");
        
        // Remove all keys for script ID 1
        size_t removed = store->remove_all(1);
        std::cout << "Removed " << removed << " keys" << std::endl;
        
        return 0;
    } catch (const keyvaluestore::KeyValueStoreError& e) {
        std::cerr << "Storage error: " << e.what() << std::endl;
        return 1;
    }
}
```

### Using SQLite Backend

```cpp
#include <keyvaluestore/KeyValueStore.hpp>
#include <iostream>

int main() {
    try {
        // Check if SQLite support is available
        #ifdef KEYVALUESTORE_USE_SQLITE
            // Create a SQLite-backed store
            auto store = keyvaluestore::KeyValueStore::createSQLite("mydb.sqlite");
            
            // Use the store just like the memory version
            store->set(1, "user_name", std::string("Alice"));
            store->set(1, "user_age", 30);
            store->set(1, "premium_member", true);
            
            // Later, retrieve the values
            if (auto name = store->get(1, "user_name")) {
                std::cout << "User: " << std::get<std::string>(name.value()) << std::endl;
            }
        #else
            std::cout << "SQLite support not available in this build" << std::endl;
        #endif
        
        return 0;
    } catch (const keyvaluestore::KeyValueStoreError& e) {
        std::cerr << "Storage error: " << e.what() << std::endl;
        return 1;
    }
}
```

### Script Isolation Example

```cpp
#include <keyvaluestore/KeyValueStore.hpp>
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
    auto store = keyvaluestore::KeyValueStore::createInMemory();
    
    // Process data for different users
    processUserData(*store, 1);
    processUserData(*store, 2);
    
    // Each user's data remains separate
    std::cout << "User 1 exists: " << store->exists(1, "last_login") << std::endl;
    std::cout << "User 2 exists: " << store->exists(2, "last_login") << std::endl;
    
    return 0;
}
```

## Build Configuration

### CMake Options

* `KEYVALUESTORE_USE_SQLITE`: Build with SQLite backend support (OFF by default)
* `KEYVALUESTORE_BUILD_TESTS`: Build the test suite (ON by default)
* `KEYVALUESTORE_BUILD_DOCS`: Build documentation (ON by default)
* `KEYVALUESTORE_BUILD_EXAMPLES`: Build example applications (ON by default)
* `KEYVALUESTORE_BUILD_COVERAGE`: Build with coverage information (OFF by default)

### Build Types

```bash
mkdir build && cd build

# Debug build (includes debug symbols, no optimization)
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build (optimized, no debug info)
cmake -DCMAKE_BUILD_TYPE=Release ..

# RelWithDebInfo (optimized, includes debug info)
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..

# Build with SQLite support
cmake -DKEYVALUESTORE_USE_SQLITE=ON ..

make
```

### Building Documentation

```bash
# Configure with documentation enabled
cmake -DKEYVALUESTORE_BUILD_DOCS=ON ..
make docs

# Open the documentation
open docs/html/index.html  # On macOS
xdg-open docs/html/index.html  # On Linux
```

### Code Coverage

To build with code coverage support:
```bash
# Configure with coverage enabled (requires Debug build)
cmake -DCMAKE_BUILD_TYPE=Debug -DKEYVALUESTORE_BUILD_COVERAGE=ON ..
make

# Build and run the tests to generate coverage data
make coverage

# Open the coverage report
open build/coverage_report/index.html  # On macOS
xdg-open build/coverage_report/index.html  # On Linux
```

Requirements for coverage:
* Debug build (`CMAKE_BUILD_TYPE=Debug`)
* gcov and lcov installed on your system
* Tests enabled (`KEYVALUESTORE_BUILD_TESTS=ON`)

Note: Coverage analysis with optimized builds may produce misleading results and is not recommended.

To install the required tools:
```bash
# Ubuntu/Debian
sudo apt-get install lcov

# macOS
brew install lcov

# Fedora
sudo dnf install lcov
```

## API Reference

The library provides a common interface through the `KeyValueStore` base class:

```cpp
class KeyValueStore {
public:
    // Factory methods
    static std::unique_ptr<KeyValueStore> createInMemory();
#ifdef KEYVALUESTORE_USE_SQLITE
    static std::unique_ptr<KeyValueStore> createSQLite(const std::string& db_path);
#endif

    // Core operations
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

* `createInMemory()`: Creates a new in-memory store instance
* `createSQLite()`: Creates a new SQLite-backed store instance (if available)
* `set()`: Store a value with the given key for a specific script
* `get()`: Retrieve a value by key for a specific script
* `exists()`: Check if a key exists for a specific script
* `remove()`: Remove a key-value pair for a specific script
* `remove_all()`: Remove all key-value pairs for a specific script

## Performance Considerations

* The in-memory implementation provides the fastest access but doesn't persist data
* The SQLite implementation (when available):
  * Uses Write-Ahead Logging (WAL) for better concurrency
  * Maintains prepared statements for common operations
  * Provides ACID guarantees for data persistence
  * Uses indices for efficient script_id-based queries

## Thread Safety

Both implementations are thread-safe:
* In-memory store uses a shared mutex for read-write synchronization
* SQLite store uses SQLite's built-in thread safety with additional mutex protection

## Error Handling

The library uses exceptions for error handling:
* All operations may throw `KeyValueStoreError` for serious errors
* Non-exceptional conditions (like key not found) use return values
* The SQLite implementation includes detailed error messages from the database

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request. For development, you can clone the repository using either:

* HTTPS: `git clone https://github.com/HarryPehkonen/KeyValueStore.git
* SSH: `git clone git@github.com:HarryPehkonen/KeyValueStore.git

## License

This project is released into the public domain - see the [LICENSE](LICENSE) file for details.
