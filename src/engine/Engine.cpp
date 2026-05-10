#include "Engine.h"

Engine::Engine()
    : pool_(4),
      taskQueue_(pool_),
      bus_(pool_),
      wal_("engine.wal") {}

std::string Engine::execute(
    const std::string& rawCommand
) {
    auto cmd =
        CommandParser::parse(rawCommand);

    if (!cmd.valid)
        return "ERR invalid command\n";

    if (cmd.name == "SET") {

        if (cmd.args.size() < 2)
            return "ERR usage: SET key value [ttl]\n";

        std::string key =
            cmd.args[0];

        std::string value =
            cmd.args[1];

        std::optional<int> ttl;

        if (cmd.args.size() >= 3) {
            ttl =
                std::stoi(cmd.args[2]);
        }

        kv_.set(
            key,
            value,
            ttl
        );

        wal_.append(
            rawCommand
        );

        bus_.publish({
            "kv.set",
            key
        });

        return "OK\n";
    }

    if (cmd.name == "GET") {

        if (cmd.args.size() != 1)
            return "ERR usage: GET key\n";

        auto val =
            kv_.get(cmd.args[0]);

        if (!val)
            return "NULL\n";

        return *val + "\n";
    }

    if (cmd.name == "DEL") {

        if (cmd.args.size() != 1)
            return "ERR usage: DEL key\n";

        bool ok =
            kv_.del(cmd.args[0]);

        if (ok) {
            wal_.append(
                rawCommand
            );

            bus_.publish({
                "kv.del",
                cmd.args[0]
            });
        }

        return ok
            ? "1\n"
            : "0\n";
    }

    if (cmd.name == "TASK") {

        if (cmd.args.empty())
            return "ERR usage: TASK name\n";

        auto id =
            taskQueue_.submit(
                [name = cmd.args[0]]() {
                    std::this_thread::sleep_for(
                        std::chrono::seconds(1)
                    );
                },
                cmd.args[0]
            );

        return id + "\n";
    }

    return "ERR unknown command\n";
}