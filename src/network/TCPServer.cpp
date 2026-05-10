#include "TCPServer.h"
#include "../engine/Engine.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

TCPServer::TCPServer(Engine& engine, int port)
    : engine_(engine), port_(port) {}

TCPServer::~TCPServer() {
    stop();
}

void TCPServer::start() {
    serverFd_ = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;

    setsockopt(
        serverFd_,
        SOL_SOCKET,
        SO_REUSEADDR,
        &opt,
        sizeof(opt)
    );

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    bind(
        serverFd_,
        (sockaddr*)&addr,
        sizeof(addr)
    );

    listen(serverFd_, 10);

    running_.store(true);

    std::cout
        << "Engine listening on port "
        << port_
        << std::endl;

    acceptLoop();
}

void TCPServer::stop() {
    running_.store(false);

    if (serverFd_ >= 0) {
        close(serverFd_);
        serverFd_ = -1;
    }

    for (auto& t : clientThreads_)
        if (t.joinable())
            t.join();
}

void TCPServer::acceptLoop() {
    while (running_.load()) {
        int clientFd = accept(
            serverFd_,
            nullptr,
            nullptr
        );

        if (clientFd < 0)
            break;

        clientThreads_.emplace_back(
            [this, clientFd]() {
                handleClient(clientFd);
            }
        );
    }
}

void TCPServer::handleClient(int fd) {
    char buf[4096];

    while (true) {
        ssize_t n = read(
            fd,
            buf,
            sizeof(buf) - 1
        );

        if (n <= 0)
            break;

        buf[n] = '\0';

        std::string cmd(buf);

        while (
            !cmd.empty() &&
            (
                cmd.back() == '\n' ||
                cmd.back() == '\r'
            )
        ) {
            cmd.pop_back();
        }

        if (cmd.empty())
            continue;

        std::string response =
            engine_.execute(cmd);

        write(
            fd,
            response.c_str(),
            response.size()
        );
    }

    close(fd);
}