#pragma once

#include <string>
#include <variant>
#include <optional>
#include <stdexcept>

namespace keyvaluestore {

// Type alias for supported value types
using ValueType = std::variant<std::string, int, double, bool>;

// Base exception class for the library
class KeyValueStoreError : public std::runtime_error {
public:
    explicit KeyValueStoreError(const std::string& what_arg) 
        : std::runtime_error(what_arg) {}
    
    explicit KeyValueStoreError(const char* what_arg) 
        : std::runtime_error(what_arg) {}
};

/**
 * @brief Abstract interface for key-value store implementations.
 * 
 * This class defines the interface for a thread-safe key-value store that supports
 * multiple value types and partitioning by script ID. Different implementations
 * can provide storage in memory, SQLite, or potentially other backends.
 */
class KeyValueStore {
public:
    virtual ~KeyValueStore() = default;

    /**
     * @brief Store a value with the given key for a specific script.
     * 
     * @param script_id Identifier for the calling script/thread
     * @param key The key under which to store the value
     * @param value The value to store (can be string, int, double, or bool)
     * @throws KeyValueStoreError if the operation fails
     */
    virtual void set(int script_id, 
                    const std::string& key, 
                    const ValueType& value) = 0;

    /**
     * @brief Retrieve a value by key for a specific script.
     * 
     * @param script_id Identifier for the calling script/thread
     * @param key The key to look up
     * @return std::optional<ValueType> The value if found, std::nullopt if not found
     * @throws KeyValueStoreError if the operation fails during lookup
     */
    virtual std::optional<ValueType> get(int script_id, 
                                       const std::string& key) = 0;

    /**
     * @brief Check if a key exists for a specific script.
     * 
     * This method is potentially more efficient than get() when you only need
     * to check for existence rather than retrieve the value.
     * 
     * @param script_id Identifier for the calling script/thread
     * @param key The key to check
     * @return bool True if the key exists, false otherwise
     * @throws KeyValueStoreError if the operation fails during lookup
     */
    virtual bool exists(int script_id, 
                       const std::string& key) = 0;

    /**
     * @brief Remove a key-value pair for a specific script.
     * 
     * @param script_id Identifier for the calling script/thread
     * @param key The key to remove
     * @return bool True if the key was found and removed, false if the key didn't exist
     * @throws KeyValueStoreError if the operation fails during removal
     */
    virtual bool remove(int script_id, 
                       const std::string& key) = 0;

    /**
     * @brief Remove all key-value pairs for a specific script.
     * 
     * This method removes all entries associated with the given script_id.
     * 
     * @param script_id Identifier for the calling script/thread
     * @return size_t The number of entries removed
     * @throws KeyValueStoreError if the operation fails during removal
     */
    virtual size_t remove_all(int script_id) = 0;

protected:
    // Protected constructor to prevent direct instantiation
    KeyValueStore() = default;
    
    // Prevent copying and moving
    KeyValueStore(const KeyValueStore&) = delete;
    KeyValueStore& operator=(const KeyValueStore&) = delete;
    KeyValueStore(KeyValueStore&&) = delete;
    KeyValueStore& operator=(KeyValueStore&&) = delete;
};

} // namespace keyvaluestore
