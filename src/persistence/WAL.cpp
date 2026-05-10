#include "WAL.h"

#include <chrono>
#include <unistd.h>
#include <cstdio>

WAL::WAL(const std::string& path)
    : path_(path) {
    file_.open(path, std::ios::app);
}

WAL::~WAL() {
    if (file_.is_open())
        file_.close();
}

void WAL::append(
    const std::string& line
) {
    std::lock_guard lock(mutex_);

    auto ts =
        std::chrono::duration_cast<
            std::chrono::milliseconds
        >(
            std::chrono::system_clock::now()
                .time_since_epoch()
        ).count();

    file_
        << line
        << " "
        << ts
        << "\n";

    file_.flush();

    FILE* fp =
        fopen(path_.c_str(), "a");

    if (fp) {
        fsync(fileno(fp));
        fclose(fp);
    }
}

void WAL::appendSet(
    const std::string& key,
    const std::string& val,
    long long ttlMs
) {
    std::string line =
        "SET " + key + " " + val;

    if (ttlMs > 0)
        line +=
            " EX " +
            std::to_string(ttlMs);

    append(line);
}

void WAL::appendDel(
    const std::string& key
) {
    append("DEL " + key);
}

std::vector<std::string>
WAL::readAll() const {
    std::lock_guard lock(mutex_);

    std::vector<std::string> lines;

    std::ifstream in(path_);

    std::string line;

    while (std::getline(in, line))
        if (!line.empty())
            lines.push_back(line);

    return lines;
}

void WAL::clear() {
    std::lock_guard lock(mutex_);

    file_.close();

    std::ofstream(
        path_,
        std::ios::trunc
    ).close();

    file_.open(
        path_,
        std::ios::app
    );
}