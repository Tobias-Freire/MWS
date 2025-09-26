#include "../headers/libtslog.h"
#include <fstream>
#include <mutex>

namespace {
    std::ofstream logFile;
    std::mutex logMutex;
}

namespace libtslog {
    void init(const std::string& filename) {
        logFile.open(filename, std::ios::app);
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(logMutex);
        logFile << message << std::endl;
    }

    void close() {
        logFile.close();
    }
}