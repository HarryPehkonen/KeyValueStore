#include "keyvaluestore/SQLiteKeyValueStore.hpp"
#include <sstream>
#include <charconv>

namespace keyvaluestore {

SQLiteKeyValueStore::SQLiteKeyValueStore(const std::string& db_path)
    : db_(nullptr, sqlite3_close)
    , stmt_set_(nullptr, sqlite3_finalize)
    , stmt_get_(nullptr, sqlite3_finalize)
    , stmt_exists_(nullptr, sqlite3_finalize)
    , stmt_remove_(nullptr, sqlite3_finalize)
    , stmt_remove_all_(nullptr, sqlite3_finalize) {
    
    sqlite3* db_raw = nullptr;
    if (sqlite3_open(db_path.c_str(), &db_raw) != SQLITE_OK) {
        throw KeyValueStoreError("Failed to open SQLite database: " + 
                               std::string(sqlite3_errmsg(db_raw)));
    }
    db_.reset(db_raw);

    // Enable WAL mode for better concurrency
    char* error_msg = nullptr;
    if (sqlite3_exec(db_.get(), "PRAGMA journal_mode=WAL;", nullptr, nullptr, &error_msg) != SQLITE_OK) {
        std::string error(error_msg);
        sqlite3_free(error_msg);
        throw KeyValueStoreError("Failed to enable WAL mode: " + error);
    }

    initializeSchema();

    // Prepare statements
    const char* sql_set = "INSERT OR REPLACE INTO key_value_store "
                         "(script_id, key, value, type) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt_raw = nullptr;
    if (sqlite3_prepare_v2(db_.get(), sql_set, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw KeyValueStoreError("Failed to prepare set statement");
    }
    stmt_set_.reset(stmt_raw);

    const char* sql_get = "SELECT value, type FROM key_value_store "
                         "WHERE script_id = ? AND key = ?";
    if (sqlite3_prepare_v2(db_.get(), sql_get, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw KeyValueStoreError("Failed to prepare get statement");
    }
    stmt_get_.reset(stmt_raw);

    const char* sql_exists = "SELECT 1 FROM key_value_store "
                           "WHERE script_id = ? AND key = ?";
    if (sqlite3_prepare_v2(db_.get(), sql_exists, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw KeyValueStoreError("Failed to prepare exists statement");
    }
    stmt_exists_.reset(stmt_raw);

    const char* sql_remove = "DELETE FROM key_value_store "
                           "WHERE script_id = ? AND key = ?";
    if (sqlite3_prepare_v2(db_.get(), sql_remove, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw KeyValueStoreError("Failed to prepare remove statement");
    }
    stmt_remove_.reset(stmt_raw);

    const char* sql_remove_all = "DELETE FROM key_value_store WHERE script_id = ?";
    if (sqlite3_prepare_v2(db_.get(), sql_remove_all, -1, &stmt_raw, nullptr) != SQLITE_OK) {
        throw KeyValueStoreError("Failed to prepare remove_all statement");
    }
    stmt_remove_all_.reset(stmt_raw);
}

SQLiteKeyValueStore::~SQLiteKeyValueStore() = default;

void SQLiteKeyValueStore::initializeSchema() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS key_value_store (
            script_id INTEGER NOT NULL,
            key TEXT NOT NULL,
            value TEXT NOT NULL,
            type CHAR(1) NOT NULL,
            PRIMARY KEY (script_id, key)
        );
        CREATE INDEX IF NOT EXISTS idx_script_id ON key_value_store(script_id);
    )";

    char* error_msg = nullptr;
    if (sqlite3_exec(db_.get(), sql, nullptr, nullptr, &error_msg) != SQLITE_OK) {
        std::string error(error_msg);
        sqlite3_free(error_msg);
        throw KeyValueStoreError("Failed to initialize database schema: " + error);
    }
}

void SQLiteKeyValueStore::set(int script_id, const std::string& key,
                             const ValueType& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto [value_str, type_indicator] = serializeValue(value);

    sqlite3_reset(stmt_set_.get());
    sqlite3_clear_bindings(stmt_set_.get());
    
    if (sqlite3_bind_int(stmt_set_.get(), 1, script_id) != SQLITE_OK ||
        sqlite3_bind_text(stmt_set_.get(), 2, key.c_str(), -1, SQLITE_STATIC) != SQLITE_OK ||
        sqlite3_bind_text(stmt_set_.get(), 3, value_str.c_str(), -1, SQLITE_STATIC) != SQLITE_OK ||
        sqlite3_bind_text(stmt_set_.get(), 4, &type_indicator, 1, SQLITE_STATIC) != SQLITE_OK) {
        throw KeyValueStoreError("Failed to bind parameters for set operation");
    }

    if (sqlite3_step(stmt_set_.get()) != SQLITE_DONE) {
        throw KeyValueStoreError("Failed to execute set operation: " + 
                               std::string(sqlite3_errmsg(db_.get())));
    }
}

std::optional<ValueType> SQLiteKeyValueStore::get(int script_id,
                                                 const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    sqlite3_reset(stmt_get_.get());
    sqlite3_clear_bindings(stmt_get_.get());

    if (sqlite3_bind_int(stmt_get_.get(), 1, script_id) != SQLITE_OK ||
        sqlite3_bind_text(stmt_get_.get(), 2, key.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        throw KeyValueStoreError("Failed to bind parameters for get operation");
    }

    if (sqlite3_step(stmt_get_.get()) == SQLITE_ROW) {
        const unsigned char* value_text = sqlite3_column_text(stmt_get_.get(), 0);
        const unsigned char* type_text = sqlite3_column_text(stmt_get_.get(), 1);
        
        if (!value_text || !type_text) {
            throw KeyValueStoreError("Retrieved null value from database");
        }

        std::string value_str(reinterpret_cast<const char*>(value_text));
        char type_indicator = *reinterpret_cast<const char*>(type_text);

        return deserializeValue(value_str, type_indicator);
    }

    return std::nullopt;
}

bool SQLiteKeyValueStore::exists(int script_id, const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    sqlite3_reset(stmt_exists_.get());
    sqlite3_clear_bindings(stmt_exists_.get());

    if (sqlite3_bind_int(stmt_exists_.get(), 1, script_id) != SQLITE_OK ||
        sqlite3_bind_text(stmt_exists_.get(), 2, key.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        throw KeyValueStoreError("Failed to bind parameters for exists operation");
    }

    return sqlite3_step(stmt_exists_.get()) == SQLITE_ROW;
}

bool SQLiteKeyValueStore::remove(int script_id, const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    sqlite3_reset(stmt_remove_.get());
    sqlite3_clear_bindings(stmt_remove_.get());

    if (sqlite3_bind_int(stmt_remove_.get(), 1, script_id) != SQLITE_OK ||
        sqlite3_bind_text(stmt_remove_.get(), 2, key.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        throw KeyValueStoreError("Failed to bind parameters for remove operation");
    }

    if (sqlite3_step(stmt_remove_.get()) != SQLITE_DONE) {
        throw KeyValueStoreError("Failed to execute remove operation");
    }

    return sqlite3_changes(db_.get()) > 0;
}

size_t SQLiteKeyValueStore::remove_all(int script_id) {
    std::lock_guard<std::mutex> lock(mutex_);

    sqlite3_reset(stmt_remove_all_.get());
    sqlite3_clear_bindings(stmt_remove_all_.get());

    if (sqlite3_bind_int(stmt_remove_all_.get(), 1, script_id) != SQLITE_OK) {
        throw KeyValueStoreError("Failed to bind parameters for remove_all operation");
    }

    if (sqlite3_step(stmt_remove_all_.get()) != SQLITE_DONE) {
        throw KeyValueStoreError("Failed to execute remove_all operation");
    }

    return static_cast<size_t>(sqlite3_changes(db_.get()));
}

std::pair<std::string, char> SQLiteKeyValueStore::serializeValue(const ValueType& value) {
    if (std::holds_alternative<std::string>(value)) {
        return {std::get<std::string>(value), 's'};
    } 
    else if (std::holds_alternative<int>(value)) {
        return {std::to_string(std::get<int>(value)), 'i'};
    }
    else if (std::holds_alternative<double>(value)) {
        // Use stringstream for consistent double formatting
        std::stringstream ss;
        ss.precision(std::numeric_limits<double>::max_digits10);
        ss << std::get<double>(value);
        return {ss.str(), 'd'};
    }
    else if (std::holds_alternative<bool>(value)) {
        return {std::get<bool>(value) ? "1" : "0", 'b'};
    }
    
    throw KeyValueStoreError("Invalid value type");
}

ValueType SQLiteKeyValueStore::deserializeValue(
    const std::string& value_str, char type_indicator) {
    
    switch (type_indicator) {
        case 's':
            return value_str;
            
        case 'i': {
            try {
                size_t pos;
                int value = std::stoi(value_str, &pos);
                if (pos != value_str.length()) {
                    throw KeyValueStoreError("Invalid integer format");
                }
                return value;
            } catch (const std::exception& e) {
                throw KeyValueStoreError("Failed to parse integer value: " + 
                                       std::string(e.what()));
            }
        }
            
        case 'd': {
            try {
                size_t pos;
                double value = std::stod(value_str, &pos);
                if (pos != value_str.length()) {
                    throw KeyValueStoreError("Invalid double format");
                }
                return value;
            } catch (const std::exception& e) {
                throw KeyValueStoreError("Failed to parse double value: " + 
                                       std::string(e.what()));
            }
        }
            
        case 'b': {
            if (value_str == "1") {
                return true;
            } else if (value_str == "0") {
                return false;
            }
            throw KeyValueStoreError("Invalid boolean value");
        }
            
        default:
            throw KeyValueStoreError("Unknown type indicator: " + 
                                   std::string(1, type_indicator));
    }
}

} // namespace keyvaluestore
