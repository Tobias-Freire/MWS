#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <string>
#include <atomic>
#include <fstream>
#include <sstream>
#include <map>
#include <sys/stat.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>

#include "./headers/conn.h"
#include "./headers/libtslog.h"

static std::atomic<bool> running{false};
static int g_serverSock = -1;
static std::mutex statsMutex;
static uint64_t totalRequests = 0;
static uint64_t totalBytesSent = 0;
static std::atomic<int> activeConnections{0};

// MIME map
static std::map<std::string,std::string> mime_map = {
    {".html","text/html"}, {".css","text/css"}, {".js","application/javascript"},
    {".png","image/png"},  {".jpg","image/jpeg"}, {".jpeg","image/jpeg"},
    {".gif","image/gif"},  {".txt","text/plain"}, {".json","application/json"},
    {".pdf","application/pdf"}
};

static std::string get_mime(const std::string &path) {
    size_t pos = path.rfind('.');
    if (pos == std::string::npos) return "application/octet-stream";
    std::string ext = path.substr(pos);
    auto it = mime_map.find(ext);
    return it != mime_map.end() ? it->second : "application/octet-stream";
}

static bool file_exists(const std::string &path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

static std::string safe_join(const std::string &base, const std::string &rel) {
    std::string p = rel;
    size_t q = p.find_first_of("?#");
    if (q != std::string::npos) p = p.substr(0,q);
    if (p == "/" || p.empty()) p = "/index.html";

    // remove ..
    while (p.find("..") != std::string::npos)
        p.erase(p.find(".."), 2);

    std::string joined = base;
    if (!joined.empty() && joined.back() == '/') joined.pop_back();
    return joined + p;
}

static void send_all(int sock, const char* buf, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t r = send(sock, buf + sent, len - sent, 0);
        if (r <= 0) break;
        sent += r;
    }
    std::lock_guard<std::mutex> lock(statsMutex);
    totalBytesSent += sent;
}

static std::string read_request(int sock) {
    std::string req;
    const int BUF = 1024;
    char buffer[BUF];
    while (req.find("\r\n\r\n") == std::string::npos &&
           req.find("\n\n") == std::string::npos) {
        ssize_t r = recv(sock, buffer, BUF, 0);
        if (r <= 0) break;
        req.append(buffer, buffer + r);
        if (req.size() > 65536) break;
    }
    return req;
}

static void handle_connection(int clientSock, sockaddr_in clientAddr, std::string wwwdir) {
    activeConnections.fetch_add(1);
    char addrStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, addrStr, sizeof(addrStr));
    int clientPort = ntohs(clientAddr.sin_port);

    libtslog::log("Accepted connection from " + std::string(addrStr) + ":" + std::to_string(clientPort));

    std::string request = read_request(clientSock);
    if (request.empty()) {
        close(clientSock);
        activeConnections.fetch_sub(1);
        return;
    }

    std::istringstream iss(request);
    std::string line;
    std::getline(iss, line);
    if (!line.empty() && line.back() == '\r') line.pop_back();

    std::string method, path, version;
    std::istringstream(line) >> method >> path >> version;

    {
        std::lock_guard<std::mutex> lock(statsMutex);
        ++totalRequests;
    }

    libtslog::log("Request: " + line);

    if (method != "GET") {
        std::string resp = "HTTP/1.0 405 Method Not Allowed\r\nConnection: close\r\nContent-Length: 0\r\n\r\n";
        send_all(clientSock, resp.c_str(), resp.size());
        close(clientSock);
        activeConnections.fetch_sub(1);
        return;
    }

    std::string fullpath = safe_join(wwwdir, path);
    if (!file_exists(fullpath)) {
        std::string body = "<html><body><h1>404 Not Found</h1></body></html>\n";
        std::ostringstream oss;
        oss << "HTTP/1.0 404 Not Found\r\n"
            << "Content-Type: text/html\r\n"
            << "Content-Length: " << body.size() << "\r\n"
            << "Connection: close\r\n\r\n" << body;
        std::string resp = oss.str();
        send_all(clientSock, resp.c_str(), resp.size());
        libtslog::log("Response: 404 for " + path);
        close(clientSock);
        activeConnections.fetch_sub(1);
        return;
    }

    std::ifstream ifs(fullpath, std::ios::binary);
    std::ostringstream buf;
    buf << ifs.rdbuf();
    std::string body = buf.str();

    std::ostringstream headers;
    headers << "HTTP/1.0 200 OK\r\n"
            << "Content-Type: " << get_mime(fullpath) << "\r\n"
            << "Content-Length: " << body.size() << "\r\n"
            << "Connection: close\r\n\r\n";
    std::string hdrs = headers.str();

    send_all(clientSock, hdrs.c_str(), hdrs.size());
    send_all(clientSock, body.data(), body.size());

    libtslog::log("Response: 200 for " + path + " (" + fullpath + ") bytes=" + std::to_string(body.size()));

    close(clientSock);
    activeConnections.fetch_sub(1);
}

int runServer(int port, const std::string &wwwdir, int backlog) {
    if (running) return -1;
    running = true;

    g_serverSock = createSocketAndBind(port);
    if (g_serverSock == -1) {
        libtslog::log("Failed to bind on port " + std::to_string(port));
        running = false;
        return -1;
    }

    if (listen(g_serverSock, backlog) == -1) {
        libtslog::log("listen() failed");
        close(g_serverSock);
        running = false;
        return -1;
    }

    std::cout << "Server listening on port " << port << " serving dir: " << wwwdir << std::endl;
    libtslog::log("HTTP server listening on port " + std::to_string(port));

    while (running) {
        sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        int clientSock = accept(g_serverSock, (sockaddr*)&clientAddr, &addrLen);
        if (clientSock == -1) {
            if (!running) break;
            continue;
        }
        std::thread t(handle_connection, clientSock, clientAddr, wwwdir);
        t.detach();
    }

    if (g_serverSock != -1) close(g_serverSock);
    g_serverSock = -1;
    running = false;
    libtslog::log("Server stopped");
    return 0;
}

void shutdownServer() {
    running = false;
    if (g_serverSock != -1) {
        close(g_serverSock);
        g_serverSock = -1;
    }
}

std::string getServerStats() {
    std::ostringstream oss;
    std::lock_guard<std::mutex> lock(statsMutex);
    oss << "total_requests: " << totalRequests << "\n"
        << "total_bytes_sent: " << totalBytesSent << "\n"
        << "active_connections: " << activeConnections.load() << "\n";
    return oss.str();
}
