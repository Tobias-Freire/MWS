#pragma once
#include <string>

namespace libtslog {
    void init(const std::string& fileName);
    void log(const std::string& message);
    void close();
}