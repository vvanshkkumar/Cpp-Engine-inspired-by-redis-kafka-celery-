#pragma once

#include <string>
#include <atomic>
#include <thread>
#include <vector>

class Engine;

class TCPServer {
public:
    explicit TCPServer(Engine& engine, int port = 6380);
    ~TCPServer();

    void start();
    void stop();

private:
    void acceptLoop();
    void handleClient(int fd);

    Engine& engine_;
    int port_;
    int serverFd_{-1};
    std::atomic<bool> running_{false};
    std::vector<std::thread> clientThreads_;
};