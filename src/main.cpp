#include <iostream>
#include <string>
#include <cstring>
#include "./headers/libtslog.h"

// forward declarations
int runServer(int port);
int runClient(const std::string &serverIp, int port, const std::string &singleMessage);

void print_usage() {
    std::cout << "Usage:\n";
    std::cout << "  program server <port>\n";
    std::cout << "  program client <ip> <port> [-m \"message\"]\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    // initialize logging (append to log.txt in project root)
    libtslog::init("log.txt");

    std::string mode = argv[1];
    if (mode == "server") {
        if (argc < 3) {
            std::cerr << "Missing port for server\n";
            return 1;
        }
        int port = std::stoi(argv[2]);
        return runServer(port);
    } else if (mode == "client") {
        if (argc < 4) {
            std::cerr << "Missing ip/port for client\n";
            return 1;
        }
        std::string ip = argv[2];
        int port = std::stoi(argv[3]);
        std::string singleMessage = "";
        
        for (int i = 4; i < argc; ++i) {
            if (std::string(argv[i]) == "-m" && i + 1 < argc) {
                singleMessage = argv[i+1];
                break;
            }
        }
        int res = runClient(ip, port, singleMessage);
        libtslog::close();
        return res;
    } else {
        print_usage();
        return 1;
    }
}
