#include "KVStore.h"

void KVStore::set(
    const std::string& key,
    const std::string& value,
    std::optional<std::chrono::milliseconds> ttl
) {
    std::unique_lock lock(mutex_);

    KVEntry entry;
    entry.value = value;

    if (ttl.has_value()) {
        entry.expiry =
            std::chrono::steady_clock::now() + ttl.value();
    }

    store_[key] = std::move(entry);
}

std::optional<std::string> KVStore::get(
    const std::string& key
) {
    std::shared_lock lock(mutex_);

    auto it = store_.find(key);

    if (it == store_.end()) {
        return std::nullopt;
    }

    if (it->second.isExpired()) {
        return std::nullopt;
    }

    return it->second.value;
}

bool KVStore::del(const std::string& key) {
    std::unique_lock lock(mutex_);

    return store_.erase(key) > 0;
}

void KVStore::purgeExpired() {
    std::vector<std::string> toDelete;

    {
        std::shared_lock lock(mutex_);

        for (auto& [key, entry] : store_) {
            if (entry.isExpired()) {
                toDelete.push_back(key);
            }
        }
    }

    if (!toDelete.empty()) {
        std::unique_lock lock(mutex_);

        for (auto& key : toDelete) {
            store_.erase(key);
        }
    }
}

size_t KVStore::size() const {
    std::shared_lock lock(mutex_);

    return store_.size();
}