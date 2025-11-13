#ifndef SANITIZER_HPP
#define SANITIZER_HPP

#include <string>
#include <regex>

namespace utils {

/**
 * @brief Input sanitization utilities
 *
 * Provides methods to sanitize and validate user input to prevent
 * XSS, SQL injection, and other security vulnerabilities.
 */
class Sanitizer {
public:
    /**
     * @brief Sanitize HTML content (escape special characters)
     * @param input Input string
     * @return Sanitized string
     */
    static std::string sanitizeHtml(const std::string& input);

    /**
     * @brief Sanitize SQL input (escape quotes)
     * @param input Input string
     * @return Sanitized string
     */
    static std::string sanitizeSql(const std::string& input);

    /**
     * @brief Remove non-alphanumeric characters
     * @param input Input string
     * @return Sanitized string
     */
    static std::string alphanumericOnly(const std::string& input);

    /**
     * @brief Validate email format
     * @param email Email address
     * @return true if valid email format
     */
    static bool isValidEmail(const std::string& email);

    /**
     * @brief Validate URL format
     * @param url URL string
     * @return true if valid URL format
     */
    static bool isValidUrl(const std::string& url);

    /**
     * @brief Remove leading/trailing whitespace
     * @param input Input string
     * @return Trimmed string
     */
    static std::string trim(const std::string& input);

    /**
     * @brief Limit string length
     * @param input Input string
     * @param max_length Maximum allowed length
     * @return Truncated string
     */
    static std::string limitLength(const std::string& input, size_t max_length);

    /**
     * @brief Remove dangerous characters for file paths
     * @param input Input string
     * @return Sanitized path
     */
    static std::string sanitizePath(const std::string& input);

    /**
     * @brief Validate integer input
     * @param input Input string
     * @return true if valid integer
     */
    static bool isValidInteger(const std::string& input);

    /**
     * @brief Validate that string contains only safe characters
     * @param input Input string
     * @return true if safe
     */
    static bool isSafeString(const std::string& input);
};

} // namespace utils

#endif // SANITIZER_HPP
