#pragma once

#include <iostream>
#include <fstream>
#include <chrono>

class Logger {
public:
    Logger(const std::string& filename) {
        // Open the file in append mode
        log_file.open(filename.c_str(), std::ofstream::out | std::ofstream::trunc);
    }

    ~Logger() {
        log_file.close();
    }

    // Log a message with the current timestamp
    void log(const std::string& message) {
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        std::tm timeinfo = {};
        localtime_s(&timeinfo, &now_c);

        log_file << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S") << " " << message << std::endl;
        log_string << std::put_time(&timeinfo, "%Y-%m-%d %H:%M:%S") << " " << message << std::endl;
    }

    std::string get_data() {
        return log_string.str();
    }

private:
    std::ofstream log_file;
    std::ostringstream log_string;
};