#include <iostream>
#include <csignal>

#include "src/engine/Engine.h"
#include "src/network/TCPServer.h"

static TCPServer* gServer = nullptr;

void signalHandler(int) {
    std::cout
        << "\nShutting down gracefully...\n";

    if (gServer)
        gServer->stop();
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    Engine::Config cfg;

    cfg.threadPoolSize = 4;
    cfg.walPath = "wal.log";

    cfg.ttlInterval =
        std::chrono::milliseconds(100);

    Engine engine(cfg);

    engine.start();

    TCPServer server(
        engine,
        6380
    );

    gServer = &server;

    server.start();

    return 0;
}