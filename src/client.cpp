#include <iostream>
#include <string>
#include <sstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

#include "./headers/libtslog.h"

int runClientHttp(const std::string &serverIp, int port, const std::string &path) {
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

    std::string reqpath = path.empty() ? "/" : path;
    if (reqpath[0] != '/') reqpath = "/" + reqpath;

    std::ostringstream req;
    req << "GET " << reqpath << " HTTP/1.0\r\n"
        << "Host: " << serverIp << "\r\n"
        << "Connection: close\r\n\r\n";

    std::string request = req.str();
    send(sock, request.c_str(), request.size(), 0);
    libtslog::log("Client sent request: " + reqpath);

    char buffer[4096];
    ssize_t r;
    while ((r = recv(sock, buffer, sizeof(buffer), 0)) > 0)
        std::cout.write(buffer, r);

    close(sock);
    libtslog::log("Client finished request: " + reqpath);
    return 0;
}
