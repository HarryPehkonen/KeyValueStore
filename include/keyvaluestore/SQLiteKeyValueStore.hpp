#pragma once

#include "KeyValueStore.hpp"
#include <sqlite3.h>
#include <memory>
#include <string>
#include <mutex>

namespace keyvaluestore {

/**
 * @class SQLiteKeyValueStore
 * @brief SQLite-based implementation of KeyValueStore
 * 
 * This class provides a persistent key-value store backed by SQLite, where each key 
 * is uniquely identified by a combination of a script ID and a string key.
 */
class SQLiteKeyValueStore : public KeyValueStore {
public:
    /**
     * @brief Constructor for SQLiteKeyValueStore
     * 
     * Opens or creates a SQLite database at the specified path and initializes
     * the required table structure.
     * 
     * @param db_path Path to the SQLite database file
     * @throws KeyValueStoreError if database initialization fails
     */
    explicit SQLiteKeyValueStore(const std::string& db_path);

    /**
     * @brief Destructor for SQLiteKeyValueStore
     * 
     * Ensures proper cleanup of SQLite resources.
     */
    ~SQLiteKeyValueStore() override;

    /**
     * @brief Sets a value for a given script ID and key
     * 
     * Stores or updates the provided value in the SQLite database, associated with
     * the specified script ID and key.
     * 
     * @param script_id Unique identifier for the script
     * @param key String key for the value
     * @param value Value to be stored
     * @throws KeyValueStoreError if the database operation fails
     * 
     * @override KeyValueStore::set
     */
    void set(int script_id, const std::string& key,
             const ValueType& value) override;

    /**
     * @brief Retrieves a value for a given script ID and key
     * 
     * Attempts to find the value associated with the specified script ID and key
     * in the SQLite database.
     * 
     * @param script_id Unique identifier for the script
     * @param key String key for the value
     * @return std::optional<ValueType> Value if found, or std::nullopt
     * @throws KeyValueStoreError if the database operation fails
     * 
     * @override KeyValueStore::get
     */
    std::optional<ValueType> get(int script_id,
                                const std::string& key) override;

    /**
     * @brief Checks if a key exists for a given script ID
     * 
     * Verifies whether a key with the specified script ID exists in the database.
     * 
     * @param script_id Unique identifier for the script
     * @param key String key to check for existence
     * @return bool True if the key exists, false otherwise
     * @throws KeyValueStoreError if the database operation fails
     * 
     * @override KeyValueStore::exists
     */
    bool exists(int script_id, const std::string& key) override;

    /**
     * @brief Removes a key-value pair for a given script ID and key
     * 
     * Attempts to delete the key-value pair from the database.
     * 
     * @param script_id Unique identifier for the script
     * @param key String key to remove
     * @return bool True if removed, false if key did not exist
     * @throws KeyValueStoreError if the database operation fails
     * 
     * @override KeyValueStore::remove
     */
    bool remove(int script_id, const std::string& key) override;

    /**
     * @brief Removes all key-value pairs for a given script ID
     * 
     * Deletes all entries for the specified script ID from the database.
     * 
     * @param script_id Unique identifier for the script
     * @return size_t Number of key-value pairs removed
     * @throws KeyValueStoreError if the database operation fails
     * 
     * @override KeyValueStore::remove_all
     */
    size_t remove_all(int script_id) override;

private:
    /**
     * @brief Initialize the database schema
     * 
     * Creates the necessary table and indexes if they don't exist.
     * 
     * @throws KeyValueStoreError if schema initialization fails
     */
    void initializeSchema();

    /**
     * @brief Convert a ValueType to its string representation for storage
     * 
     * @param value The value to convert
     * @return std::pair<std::string, char> The string value and type indicator
     */
    static std::pair<std::string, char> serializeValue(const ValueType& value);

    /**
     * @brief Convert stored string and type indicator back to ValueType
     * 
     * @param value_str The stored string value
     * @param type_indicator The type indicator character
     * @return ValueType The reconstructed value
     * @throws KeyValueStoreError if value cannot be deserialized
     */
    static ValueType deserializeValue(const std::string& value_str, char type_indicator);

    std::unique_ptr<sqlite3, decltype(&sqlite3_close)> db_;
    mutable std::mutex mutex_;
    
    // Prepared statements for common operations
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt_set_;
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt_get_;
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt_exists_;
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt_remove_;
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt_remove_all_;
};

} // namespace keyvaluestore
