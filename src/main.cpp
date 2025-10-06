#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstring>
#include <unistd.h>
#include "./headers/libtslog.h"

// forward declarations implemented in server.cpp and client.cpp
int runServer(int port, const std::string &wwwdir, int backlog);
void shutdownServer();
std::string getServerStats();
int runClientHttp(const std::string &serverIp, int port, const std::string &path);

void print_usage() {
    std::cout << "Usage:\n";
    std::cout << "  program server <port> [www_dir] [backlog]\n";
    std::cout << "  program client <ip> <port> <path>\n\n";
    std::cout << "Server commands (stdin when server running):\n";
    std::cout << "  stats   - print server statistics\n";
    std::cout << "  stop    - stop the server gracefully\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string mode = argv[1];
    if (mode == "server") {
        if (argc < 3) {
            std::cerr << "Missing port for server\n";
            print_usage();
            return 1;
        }
        int port = std::stoi(argv[2]);
        std::string wwwdir = "www";
        int backlog = 10;
        if (argc >= 4) wwwdir = argv[3];
        if (argc >= 5) backlog = std::stoi(argv[4]);

        libtslog::init("log.txt");
        libtslog::log("Starting HTTP server on port " + std::to_string(port));

        // Rodar servidor no mesmo thread se stdin não estiver interativo
        if (!isatty(fileno(stdin))) {
            // Executando via nohup ou make test → bloqueia até shutdownServer()
            runServer(port, wwwdir, backlog);
        } else {
            // Modo interativo → servidor em thread separada + CLI
            std::thread serverThread([&](){
                runServer(port, wwwdir, backlog);
            });
            serverThread.detach();

            std::string cmd;
            while (true) {
                if (!std::getline(std::cin, cmd)) break;
                if (cmd == "stats") {
                    std::cout << getServerStats() << std::endl;
                } else if (cmd == "stop") {
                    libtslog::log("Received stop command from CLI");
                    shutdownServer();
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    break;
                } else if (!cmd.empty()) {
                    std::cout << "Unknown command\n";
                }
            }
        }

        libtslog::log("Server main exiting");
        libtslog::close();
        return 0;
    }

    else if (mode == "client") {
        if (argc < 5) {
            std::cerr << "Missing ip/port/path for client\n";
            print_usage();
            return 1;
        }
        std::string ip = argv[2];
        int port = std::stoi(argv[3]);
        std::string path = argv[4];

        libtslog::init("log.txt");
        int res = runClientHttp(ip, port, path);
        libtslog::close();
        return res;
    }

    else {
        print_usage();
        return 1;
    }
}
