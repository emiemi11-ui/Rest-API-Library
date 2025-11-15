#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <sstream>
#include <chrono>
#include <iomanip>

namespace utils {

/**
 * @brief Thread-safe logging system with file rotation
 *
 * Provides different log levels and automatic file rotation.
 */
class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    /**
     * @brief Get singleton instance
     * @return Logger instance
     */
    static Logger& getInstance();

    /**
     * @brief Initialize logger
     * @param log_file Path to log file
     * @param max_size Maximum file size before rotation (in bytes)
     * @param max_files Maximum number of rotated files to keep
     */
    void initialize(const std::string& log_file = "rest_api.log",
                   size_t max_size = 10 * 1024 * 1024,  // 10MB
                   int max_files = 5);

    /**
     * @brief Set minimum log level
     * @param level Minimum level to log
     */
    void setLevel(Level level);

    /**
     * @brief Log debug message
     */
    void debug(const std::string& message);

    /**
     * @brief Log info message
     */
    void info(const std::string& message);

    /**
     * @brief Log warning message
     */
    void warning(const std::string& message);

    /**
     * @brief Log error message
     */
    void error(const std::string& message);

    /**
     * @brief Log critical message
     */
    void critical(const std::string& message);

    /**
     * @brief Flush logs to file
     */
    void flush();

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string log_file_;
    std::ofstream file_;
    std::mutex mutex_;
    Level min_level_;
    size_t max_size_;
    size_t current_size_;
    int max_files_;

    void log(Level level, const std::string& message);
    void rotate();
    std::string levelToString(Level level);
    std::string getCurrentTimestamp();
    std::string formatMessage(Level level, const std::string& message);
};

// Convenience macros
#define LOG_DEBUG(msg) utils::Logger::getInstance().debug(msg)
#define LOG_INFO(msg) utils::Logger::getInstance().info(msg)
#define LOG_WARNING(msg) utils::Logger::getInstance().warning(msg)
#define LOG_ERROR(msg) utils::Logger::getInstance().error(msg)
#define LOG_CRITICAL(msg) utils::Logger::getInstance().critical(msg)

} // namespace utils

#endif // LOGGER_HPP
