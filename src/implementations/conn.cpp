#include "../headers/conn.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int createSocketAndBind(int port) {
    // AF_INET for IPv4, SOCK_STREAM for TCP
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        return -1; // Error creating socket
    }

    // Define server address
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the specified port
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        // Port already in use or other error
        close(serverSocket);
        return -1;
    }

    return serverSocket;
}