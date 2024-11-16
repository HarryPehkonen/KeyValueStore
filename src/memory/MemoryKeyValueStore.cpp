#include "keyvaluestore/KeyValueStore.hpp"
#include "keyvaluestore/MemoryKeyValueStore.hpp"

#ifdef KEYVALUESTORE_USE_SQLITE
#include "keyvaluestore/SQLiteKeyValueStore.hpp"
#endif

namespace keyvaluestore {

std::unique_ptr<KeyValueStore> KeyValueStore::createInMemory() {
    return std::make_unique<MemoryKeyValueStore>();
}

#ifdef KEYVALUESTORE_USE_SQLITE
std::unique_ptr<KeyValueStore> KeyValueStore::createSQLite(const std::string& db_path) {
    return std::make_unique<SQLiteKeyValueStore>(db_path);
}
#endif

} // namespace keyvaluestore
