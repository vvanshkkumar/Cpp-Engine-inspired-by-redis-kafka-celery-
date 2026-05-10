#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <vector>

class WAL {
public:
    explicit WAL(const std::string& path);
    ~WAL();

    void appendSet(
        const std::string& key,
        const std::string& val,
        long long ttlMs = -1
    );

    void appendDel(
        const std::string& key
    );

    std::vector<std::string> readAll() const;

    void clear();

private:
    void append(
        const std::string& line
    );

    std::string path_;
    mutable std::ofstream file_;
    mutable std::mutex mutex_;
};