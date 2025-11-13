#include "../../include/utils/logger.hpp"
#include <iostream>
#include <ctime>
#include <sys/stat.h>

namespace utils {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() : min_level_(Level::INFO), current_size_(0), max_size_(10 * 1024 * 1024), max_files_(5) {}

Logger::~Logger() {
    if (file_.is_open()) {
        file_.close();
    }
}

void Logger::initialize(const std::string& log_file, size_t max_size, int max_files) {
    std::lock_guard<std::mutex> lock(mutex_);

    log_file_ = log_file;
    max_size_ = max_size;
    max_files_ = max_files;

    file_.open(log_file_, std::ios::app);
    if (!file_.is_open()) {
        std::cerr << "Failed to open log file: " << log_file << std::endl;
        return;
    }

    // Get current file size
    struct stat st;
    if (stat(log_file_.c_str(), &st) == 0) {
        current_size_ = st.st_size;
    }
}

void Logger::setLevel(Level level) {
    min_level_ = level;
}

void Logger::debug(const std::string& message) {
    log(Level::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(Level::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(Level::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(Level::ERROR, message);
}

void Logger::critical(const std::string& message) {
    log(Level::CRITICAL, message);
}

void Logger::log(Level level, const std::string& message) {
    if (level < min_level_) {
        return;
    }

    std::string formatted = formatMessage(level, message);

    std::lock_guard<std::mutex> lock(mutex_);

    // Check if rotation is needed
    if (current_size_ + formatted.length() > max_size_) {
        rotate();
    }

    if (file_.is_open()) {
        file_ << formatted << std::endl;
        current_size_ += formatted.length() + 1;
    }

    // Also output to console for ERROR and CRITICAL
    if (level >= Level::ERROR) {
        std::cerr << formatted << std::endl;
    }
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_.is_open()) {
        file_.flush();
    }
}

void Logger::rotate() {
    if (file_.is_open()) {
        file_.close();
    }

    // Rotate existing files
    for (int i = max_files_ - 1; i >= 1; --i) {
        std::string old_file = log_file_ + "." + std::to_string(i);
        std::string new_file = log_file_ + "." + std::to_string(i + 1);
        rename(old_file.c_str(), new_file.c_str());
    }

    // Rename current log file
    std::string backup = log_file_ + ".1";
    rename(log_file_.c_str(), backup.c_str());

    // Open new log file
    file_.open(log_file_, std::ios::app);
    current_size_ = 0;
}

std::string Logger::levelToString(Level level) {
    switch (level) {
        case Level::DEBUG: return "DEBUG";
        case Level::INFO: return "INFO";
        case Level::WARNING: return "WARNING";
        case Level::ERROR: return "ERROR";
        case Level::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm tm_buf;
    localtime_r(&time_t_now, &tm_buf);

    std::ostringstream ss;
    ss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();

    return ss.str();
}

std::string Logger::formatMessage(Level level, const std::string& message) {
    std::ostringstream ss;
    ss << "[" << getCurrentTimestamp() << "] ";
    ss << "[" << levelToString(level) << "] ";
    ss << message;
    return ss.str();
}

} // namespace utils
