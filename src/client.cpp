#include <iostream>
#include <thread>
#include <string>
#include <atomic>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

#include "./headers/libtslog.h"

static std::atomic<bool> running{true};

void receiver_thread(int sock) {
    const int BUF_SIZE = 1024;
    char buffer[BUF_SIZE];
    while (running) {
        ssize_t received = recv(sock, buffer, BUF_SIZE - 1, 0);
        if (received > 0) {
            buffer[received] = '\0';
            std::cout << "[remote] " << buffer;
        } else {
            std::cout << "Connection closed.\n";
            running = false;
            break;
        }
    }
}

int runClient(const std::string &serverIp, int port, const std::string &singleMessage) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Failed to create socket\n";
        return -1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr) != 1) {
        std::cerr << "Invalid server IP\n";
        close(sock);
        return -1;
    }

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Failed to connect to server\n";
        close(sock);
        return -1;
    }

    std::string connLog = std::string("[CLIENT] Connected to ") + serverIp + ":" + std::to_string(port);
    libtslog::log(connLog);
    std::cout << connLog << std::endl;

    std::thread recvT(receiver_thread, sock);

    if (!singleMessage.empty()) {
        std::string msg = singleMessage + "\n";
        send(sock, msg.c_str(), msg.size(), 0);
        libtslog::log(std::string("[CLIENT] Sent (single): ") + msg);
        running = false;
    } else {
        std::string line;
        while (running && std::getline(std::cin, line)) {
            if (line == "/quit") {
                send(sock, line.c_str(), line.size(), 0);
                running = false;
                break;
            }
            line += "\n";
            send(sock, line.c_str(), line.size(), 0);
            libtslog::log(std::string("[CLIENT] Sent: ") + line);
        }
    }

    if (recvT.joinable()) recvT.join();
    close(sock);
    libtslog::log("[CLIENT] Client exiting");
    return 0;
}
