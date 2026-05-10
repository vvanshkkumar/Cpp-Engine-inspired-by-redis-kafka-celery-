#include "Engine.h"

#include "../network/CommandParser.h"

#include <iostream>

Engine::Engine(Config cfg)
    : cfg_(cfg)
    , wal_(cfg.walPath)
    , ttl_(kv_, cfg.ttlInterval)
    , pool_(cfg.threadPoolSize)
    , queue_(pool_)
    , bus_(pool_) {}

Engine::~Engine() {
    stop();
}

void Engine::start() {
    recoverFromWAL();

    ttl_.start();

    bus_.subscribe(
        "key.expired",
        [](const Message& m) {
        }
    );

    std::cout
        << "Engine started."
        << std::endl;
}

void Engine::stop() {
    ttl_.stop();

    pool_.shutdown();
}

void Engine::recoverFromWAL() {
    auto lines =
        wal_.readAll();

    if (lines.empty())
        return;

    std::cout
        << "Recovering "
        << lines.size()
        << " entries from WAL..."
        << std::endl;

    for (auto& line : lines) {
        auto cmd =
            CommandParser::parse(line);

        if (
            cmd.name == "SET"
            &&
            cmd.args.size() >= 2
        ) {
            std::optional<
                std::chrono::milliseconds
            > ttl;

            for (
                size_t i = 2;
                i + 1 < cmd.args.size();
                i++
            ) {
                if (
                    cmd.args[i]
                    == "EX"
                ) {
                    ttl =
                        std::chrono::milliseconds(
                            std::stoll(
                                cmd.args[i + 1]
                            )
                        );
                }
            }

            kv_.set(
                cmd.args[0],
                cmd.args[1],
                ttl
            );

        } else if (
            cmd.name == "DEL"
            &&
            !cmd.args.empty()
        ) {
            kv_.del(
                cmd.args[0]
            );
        }
    }

    std::cout
        << "Recovery complete."
        << std::endl;
}

std::string Engine::execute(
    const std::string& raw
) {
    auto cmd =
        CommandParser::parse(raw);

    if (!cmd.valid)
        return
            "-ERR Invalid command\n";

    if (cmd.name == "SET") {

        if (
            cmd.args.size() < 2
        ) {
            return
                "-ERR SET needs key value\n";
        }

        std::optional<
            std::chrono::milliseconds
        > ttl;

        long long ttlMs = -1;

        for (
            size_t i = 2;
            i + 1 < cmd.args.size();
            i++
        ) {
            if (
                cmd.args[i]
                == "EX"
            ) {
                ttlMs =
                    std::stoll(
                        cmd.args[i + 1]
                    );

                ttl =
                    std::chrono::milliseconds(
                        ttlMs
                    );
            }
        }

        wal_.appendSet(
            cmd.args[0],
            cmd.args[1],
            ttlMs
        );

        kv_.set(
            cmd.args[0],
            cmd.args[1],
            ttl
        );

        return "+OK\n";
    }

    if (cmd.name == "GET") {

        if (
            cmd.args.empty()
        ) {
            return
                "-ERR GET needs key\n";
        }

        auto val =
            kv_.get(
                cmd.args[0]
            );

        return val
            ? "+VALUE " +
                  *val + "\n"
            : "-ERR Not found\n";
    }

    if (cmd.name == "DEL") {

        if (
            cmd.args.empty()
        ) {
            return
                "-ERR DEL needs key\n";
        }

        wal_.appendDel(
            cmd.args[0]
        );

        return kv_.del(
            cmd.args[0]
        )
            ? "+OK\n"
            : "-ERR Not found\n";
    }

    if (
        cmd.name
        == "PUBLISH"
    ) {

        if (
            cmd.args.size() < 2
        ) {
            return
                "-ERR PUBLISH needs topic msg\n";
        }

        bus_.publish({
            cmd.args[0],
            cmd.args[1],
            "client"
        });

        return "+OK\n";
    }

    if (
        cmd.name
        == "PING"
    ) {
        return "+PONG\n";
    }

    return
        "-ERR Unknown: "
        + cmd.name
        + "\n";
}