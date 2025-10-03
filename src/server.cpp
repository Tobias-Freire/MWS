#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <string>
#include <atomic>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

#include "./headers/conn.h"
#include "./headers/libtslog.h"

static std::vector<int> clients;
static std::mutex clientsMutex;
static std::atomic<bool> running{true};

void broadcast_message(const std::string &msg, int senderSock) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (int sock : clients) {
        if (sock == senderSock) continue;
        send(sock, msg.c_str(), msg.size(), 0);
    }
}

void handle_client(int clientSock, sockaddr_in clientAddr) {
    char addrStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, addrStr, sizeof(addrStr));
    int clientPort = ntohs(clientAddr.sin_port);

    std::string connMsg = std::string("[SERVER] Client connected: ") + addrStr + ":" + std::to_string(clientPort);
    libtslog::log(connMsg);
    std::cout << connMsg << std::endl;

    const int BUF_SIZE = 1024;
    char buffer[BUF_SIZE];

    while (running) {
        ssize_t received = recv(clientSock, buffer, BUF_SIZE - 1, 0);
        if (received > 0) {
            buffer[received] = '\0';
            std::string msg(buffer);
            while (!msg.empty() && (msg.back() == '\n' || msg.back() == '\r')) msg.pop_back();

            if (msg == "/quit") break;

            std::string logMsg = std::string(addrStr) + ":" + std::to_string(clientPort) + " -> " + msg;
            libtslog::log(logMsg);
            std::cout << logMsg << std::endl;

            std::string toSend = std::string(addrStr) + ":" + std::to_string(clientPort) + " says: " + msg + "\n";
            broadcast_message(toSend, clientSock);
        } else {
            break;
        }
    }

    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(std::remove(clients.begin(), clients.end(), clientSock), clients.end());
    }
    close(clientSock);

    std::string disconnMsg = std::string("[SERVER] Client disconnected: ") + addrStr + ":" + std::to_string(clientPort);
    libtslog::log(disconnMsg);
    std::cout << disconnMsg << std::endl;
}

int runServer(int port) {
    int serverSock = createSocketAndBind(port);
    if (serverSock == -1) {
        std::cerr << "[SERVER] Failed to create/bind server socket on port " << port << std::endl;
        return -1;
    }

    if (listen(serverSock, 10) == -1) {
        std::cerr << "listen() failed\n";
        close(serverSock);
        return -1;
    }

    std::cout << "Server listening on port " << port << std::endl;
    libtslog::log(std::string("[SERVER] Server started on port ") + std::to_string(port));

    while (running) {
        sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        int clientSock = accept(serverSock, (sockaddr*)&clientAddr, &addrLen);
        if (clientSock == -1) continue;

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back(clientSock);
        }

        std::thread t(handle_client, clientSock, clientAddr);
        t.detach();
    }

    close(serverSock);
    return 0;
}
