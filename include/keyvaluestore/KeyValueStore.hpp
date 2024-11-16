#pragma once

#include <string>
#include <variant>
#include <optional>
#include <stdexcept>
#include <memory>

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
 * @details The KeyValueStore class provides a common interface for different
 * storage backend implementations. By default, an in-memory store is always
 * available. Additional storage backends may be available depending on compile-time
 * options:
 *
 * Available storage backends:
 * - Memory (always available): Fast, non-persistent in-memory storage
 * @if KEYVALUESTORE_USE_SQLITE
 * - SQLite: Persistent storage using SQLite database backend
 * @endif
 *
 * @note Storage backend availability is determined at compile time. Use the
 * appropriate CMake options to enable or disable specific backends:
 * - KEYVALUESTORE_USE_SQLITE=ON/OFF: Enable/disable SQLite backend
 */
class KeyValueStore {
public:
    /**
     * @brief Creates an in-memory key-value store
     * 
     * Creates a new instance of a memory-based key-value store. This store type
     * is always available but does not persist data between program executions.
     *
     * @return std::unique_ptr<KeyValueStore> Pointer to the created store
     */
    static std::unique_ptr<KeyValueStore> createInMemory();

#ifdef KEYVALUESTORE_USE_SQLITE
    /**
     * @brief Creates a SQLite-backed key-value store
     * 
     * Creates a new instance of a SQLite-backed key-value store. This store type
     * provides persistent storage using SQLite as the backend database.
     *
     * This function is only available when the library is compiled with
     * SQLite support (KEYVALUESTORE_USE_SQLITE=ON).
     * 
     * @param db_path Path to SQLite database file
     * @return std::unique_ptr<KeyValueStore> Pointer to the created store
     * @throws KeyValueStoreError if database operations fail
     */
    static std::unique_ptr<KeyValueStore> createSQLite(const std::string& db_path);
#endif

    virtual ~KeyValueStore() = default;

    /**
     * @brief Sets a value for a given script ID and key
     * 
     * @param script_id Unique identifier for the script
     * @param key String key for the value
     * @param value Value to be stored
     * @throws KeyValueStoreError if the storage operation fails
     */
    virtual void set(int script_id, 
                    const std::string& key, 
                    const ValueType& value) = 0;
    virtual std::optional<ValueType> get(int script_id, 
                                       const std::string& key) = 0;

    virtual bool exists(int script_id, 
                       const std::string& key) = 0;

    virtual bool remove(int script_id, 
                       const std::string& key) = 0;

    virtual size_t remove_all(int script_id) = 0;

protected:
    KeyValueStore() = default;
    
    KeyValueStore(const KeyValueStore&) = delete;
    KeyValueStore& operator=(const KeyValueStore&) = delete;
    KeyValueStore(KeyValueStore&&) = delete;
    KeyValueStore& operator=(KeyValueStore&&) = delete;
};

} // namespace keyvaluestore
