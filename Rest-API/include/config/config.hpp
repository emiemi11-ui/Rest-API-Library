#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <map>
#include <fstream>
#include <stdexcept>

namespace config {

/**
 * @brief Configuration manager for REST API server
 *
 * Loads configuration from JSON file and environment variables.
 */
class Config {
public:
    /**
     * @brief Get singleton instance
     */
    static Config& getInstance();

    /**
     * @brief Load configuration from file
     * @param config_file Path to JSON config file
     * @return true if successful, false otherwise
     */
    bool loadFromFile(const std::string& config_file);

    /**
     * @brief Get string configuration value
     */
    std::string getString(const std::string& key, const std::string& default_value = "") const;

    /**
     * @brief Get integer configuration value
     */
    int getInt(const std::string& key, int default_value = 0) const;

    /**
     * @brief Get boolean configuration value
     */
    bool getBool(const std::string& key, bool default_value = false) const;

    /**
     * @brief Get double configuration value
     */
    double getDouble(const std::string& key, double default_value = 0.0) const;

    /**
     * @brief Set configuration value
     */
    void set(const std::string& key, const std::string& value);

    /**
     * @brief Check if key exists
     */
    bool has(const std::string& key) const;

    // Server configuration
    int getServerPort() const { return getInt("server.port", 8080); }
    int getWorkerCount() const { return getInt("server.workers", 4); }
    int getThreadsPerWorker() const { return getInt("server.threads_per_worker", 8); }

    // Database configuration
    std::string getDatabasePath() const { return getString("database.path", "rest_api.db"); }
    int getConnectionPoolMin() const { return getInt("database.pool.min", 5); }
    int getConnectionPoolMax() const { return getInt("database.pool.max", 20); }

    // JWT configuration
    std::string getJwtSecret() const { return getString("jwt.secret", "default_secret_change_me"); }
    int getJwtExpirationHours() const { return getInt("jwt.expiration_hours", 24); }

    // Rate limiting configuration
    bool isRateLimitingEnabled() const { return getBool("rate_limiting.enabled", true); }
    int getRateLimitMaxRequests() const { return getInt("rate_limiting.max_requests", 100); }
    int getRateLimitWindowSeconds() const { return getInt("rate_limiting.window_seconds", 60); }

    // Logging configuration
    std::string getLogFile() const { return getString("logging.file", "rest_api.log"); }
    std::string getLogLevel() const { return getString("logging.level", "INFO"); }
    size_t getLogMaxSize() const { return static_cast<size_t>(getInt("logging.max_size_mb", 10)) * 1024 * 1024; }
    int getLogMaxFiles() const { return getInt("logging.max_files", 5); }

    // CORS configuration
    bool isCorsEnabled() const { return getBool("cors.enabled", true); }
    std::string getCorsOrigin() const { return getString("cors.origin", "*"); }

private:
    Config();
    ~Config() = default;
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    std::map<std::string, std::string> config_;

    void loadDefaults();
    void loadEnvironmentVariables();
    std::string trim(const std::string& str);
};

} // namespace config

#endif // CONFIG_HPP
