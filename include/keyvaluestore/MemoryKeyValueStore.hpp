#pragma once

#include "KeyValueStore.hpp"
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace keyvaluestore {

/**
 * @class MemoryKeyValueStore
 * @brief In-memory implementation of KeyValueStore using std::unordered_map
 * 
 * This class provides a thread-safe, in-memory key-value store, where each key is 
 * uniquely identified by a combination of a script ID and a string key.
 */
class MemoryKeyValueStore : public KeyValueStore {
public:
    /**
     * @brief Default constructor for MemoryKeyValueStore
     * 
     * Initializes an empty in-memory key-value store.
     * 
     * @exception None (noexcept)
     */
    MemoryKeyValueStore() noexcept = default;

    /**
     * @brief Sets a value for a given script ID and key
     * 
     * Stores the provided value in the in-memory store, associated with the 
     * specified script ID and key. If the key already exists, its value is updated.
     * 
     * @param script_id Unique identifier for the script
     * @param key String key for the value
     * @param value Value to be stored
     * 
     * @override KeyValueStore::set
     */
    void set(int script_id, const std::string& key,
             const ValueType& value) override {
        std::unique_lock lock(mutex_);
        store_[makeKey(script_id, key)] = value;
    }

    /**
     * @brief Retrieves a value for a given script ID and key
     * 
     * Attempts to find the value associated with the specified script ID and key 
     * in the in-memory store. If found, returns the value; otherwise, returns an 
     * empty optional.
     * 
     * @param script_id Unique identifier for the script
     * @param key String key for the value
     * 
     * @return std::optional<ValueType> Value if found, or std::nullopt
     * 
     * @override KeyValueStore::get
     */
    std::optional<ValueType> get(int script_id,
                                const std::string& key) override {
        std::shared_lock lock(mutex_);
        auto it = store_.find(makeKey(script_id, key));
        if (it!= store_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    /**
     * @brief Checks if a key exists for a given script ID
     * 
     * Verifies whether a key with the specified script ID and name is present in 
     * the in-memory store.
     * 
     * @param script_id Unique identifier for the script
     * @param key String key to check for existence
     * 
     * @return bool True if the key exists, false otherwise
     * 
     * @override KeyValueStore::exists
     */
    bool exists(int script_id, const std::string& key) override {
        std::shared_lock lock(mutex_);
        return store_.contains(makeKey(script_id, key));
    }

    /**
     * @brief Removes a key-value pair for a given script ID and key
     * 
     * Attempts to delete the key-value pair associated with the specified script 
     * ID and key from the in-memory store. Returns whether the removal was successful.
     * 
     * @param script_id Unique identifier for the script
     * @param key String key to remove
     * 
     * @return bool True if removed, false if key did not exist
     * 
     * @override KeyValueStore::remove
     */
    bool remove(int script_id, const std::string& key) override {
        std::unique_lock lock(mutex_);
        return store_.erase(makeKey(script_id, key)) > 0;
    }

    /**
     * @brief Removes all key-value pairs for a given script ID
     * 
     * Deletes all key-value pairs associated with the specified script ID from 
     * the in-memory store. Returns the number of removed pairs.
     * 
     * @param script_id Unique identifier for the script
     * 
     * @return size_t Number of key-value pairs removed
     * 
     * @override KeyValueStore::remove_all
     */
    size_t remove_all(int script_id) override {
        std::unique_lock lock(mutex_);
        size_t count = 0;
        auto it = store_.begin();
        while (it!= store_.end()) {
            if (it->first.first == script_id) {
                it = store_.erase(it);
                ++count;
            } else {
                ++it;
            }
        }
        return count;
    }

private:
    /**
     * @typedef KeyType
     * @brief Type definition for a key (script ID, string key pair)
     */
    using KeyType = std::pair<int, std::string>;

    /**
     * @struct KeyHash
     * @brief Custom hash structure for KeyType
     * 
     * Provides a hash function for KeyType, combining the hashes of the script ID 
     * and the string key.
     */
    struct KeyHash {
        /**
         * @brief Hash function for KeyType
         * 
         * Combines the hashes of the script ID and the string key using XOR and 
         * bit shift operations.
         * 
         * @param k KeyType to hash
         * 
         * @return std::size_t Hash value
         */
        std::size_t operator()(const KeyType& k) const {
            return std::hash<int>{}(k.first) ^
                   (std::hash<std::string>{}(k.second) << 1);
        }
    };

    /**
     * @brief Creates a KeyType from a script ID and a string key
     * 
     * Convenience function to create a KeyType pair from the given script ID and 
     * string key.
     * 
     * @param script_id Unique identifier for the script
     * @param key String key
     * 
     * @return KeyType Script ID and key pair
     */
    static KeyType makeKey(int script_id, const std::string& key) {
        return std::make_pair(script_id, key);
    }

    /**
     * @var mutex_
     * @brief Shared mutex for thread-safe access to the store
     */
    mutable std::shared_mutex mutex_;

    /**
     * @var store_
     * @brief In-memory key-value store using std::unordered_map
     * 
     * Stores key-value pairs, where each key is a unique combination of a script 
     * ID and a string key, and values are of type ValueType.
     */
    std::unordered_map<KeyType, ValueType, KeyHash> store_;
};

} // namespace keyvaluestore
