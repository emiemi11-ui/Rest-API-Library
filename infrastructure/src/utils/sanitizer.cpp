#include "../../include/utils/sanitizer.hpp"
#include <algorithm>
#include <cctype>

namespace utils {

std::string Sanitizer::sanitizeHtml(const std::string& input) {
    std::string result;
    result.reserve(input.length());

    for (char c : input) {
        switch (c) {
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '&': result += "&amp;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&#x27;"; break;
            case '/': result += "&#x2F;"; break;
            default: result += c;
        }
    }

    return result;
}

std::string Sanitizer::sanitizeSql(const std::string& input) {
    std::string result;
    result.reserve(input.length());

    for (char c : input) {
        if (c == '\'') {
            result += "''";  // Escape single quotes
        } else if (c == '\\') {
            result += "\\\\";  // Escape backslashes
        } else {
            result += c;
        }
    }

    return result;
}

std::string Sanitizer::alphanumericOnly(const std::string& input) {
    std::string result;
    result.reserve(input.length());

    for (char c : input) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            result += c;
        }
    }

    return result;
}

bool Sanitizer::isValidEmail(const std::string& email) {
    // Simple email validation
    std::regex email_regex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return std::regex_match(email, email_regex);
}

bool Sanitizer::isValidUrl(const std::string& url) {
    // Simple URL validation
    std::regex url_regex(R"(^(https?:\/\/)?([\da-z\.-]+)\.([a-z\.]{2,6})([\/\w \.-]*)*\/?$)");
    return std::regex_match(url, url_regex);
}

std::string Sanitizer::trim(const std::string& input) {
    size_t start = 0;
    size_t end = input.length();

    while (start < end && std::isspace(static_cast<unsigned char>(input[start]))) {
        ++start;
    }

    while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) {
        --end;
    }

    return input.substr(start, end - start);
}

std::string Sanitizer::limitLength(const std::string& input, size_t max_length) {
    if (input.length() <= max_length) {
        return input;
    }
    return input.substr(0, max_length);
}

std::string Sanitizer::sanitizePath(const std::string& input) {
    std::string result;
    result.reserve(input.length());

    for (char c : input) {
        // Remove dangerous characters for file paths
        if (c == '.' && result.length() > 0 && result.back() == '.') {
            continue;  // Prevent ".." directory traversal
        }
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
            continue;  // Remove path separator and dangerous characters
        }
        result += c;
    }

    return result;
}

bool Sanitizer::isValidInteger(const std::string& input) {
    if (input.empty()) return false;

    size_t start = 0;
    if (input[0] == '-' || input[0] == '+') {
        start = 1;
    }

    if (start >= input.length()) return false;

    for (size_t i = start; i < input.length(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(input[i]))) {
            return false;
        }
    }

    return true;
}

bool Sanitizer::isSafeString(const std::string& input) {
    for (char c : input) {
        unsigned char uc = static_cast<unsigned char>(c);

        // Allow alphanumeric, space, and common safe punctuation
        if (std::isalnum(uc) || uc == ' ' || uc == '.' || uc == ',' ||
            uc == '-' || uc == '_' || uc == '@' || uc == '#') {
            continue;
        }

        // Reject control characters and other potentially dangerous characters
        if (uc < 32 || uc == 127) {
            return false;
        }
    }

    return true;
}

} // namespace utils
