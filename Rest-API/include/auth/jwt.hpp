#ifndef JWT_HPP
#define JWT_HPP

#include <string>
#include <ctime>
#include <map>
#include <sstream>
#include <iomanip>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include "../utils/json.hpp"

namespace auth {

/**
 * @brief Simple JWT (JSON Web Token) implementation for authentication
 *
 * Provides token generation and validation for secure API authentication.
 * Uses HMAC-SHA256 for signing tokens.
 */
class JWT {
public:
    /**
     * @brief Constructor
     * @param secret Secret key used for signing tokens
     * @param expiration_hours Token expiration time in hours (default: 24)
     */
    explicit JWT(const std::string& secret, int expiration_hours = 24);

    /**
     * @brief Generate JWT token
     * @param user_id User identifier
     * @param username Username
     * @param role User role (admin, user, guest, etc.)
     * @return JWT token string
     */
    std::string generateToken(int user_id, const std::string& username, const std::string& role = "user");

    /**
     * @brief Validate JWT token
     * @param token JWT token to validate
     * @return true if valid, false otherwise
     */
    bool validateToken(const std::string& token);

    /**
     * @brief Extract user ID from token
     * @param token JWT token
     * @return User ID, or -1 if invalid
     */
    int getUserIdFromToken(const std::string& token);

    /**
     * @brief Extract username from token
     * @param token JWT token
     * @return Username, or empty string if invalid
     */
    std::string getUsernameFromToken(const std::string& token);

    /**
     * @brief Extract role from token
     * @param token JWT token
     * @return Role, or empty string if invalid
     */
    std::string getRoleFromToken(const std::string& token);

    /**
     * @brief Check if token is expired
     * @param token JWT token
     * @return true if expired, false otherwise
     */
    bool isTokenExpired(const std::string& token);

private:
    std::string secret_;
    int expiration_hours_;

    // Helper methods
    std::string base64Encode(const unsigned char* buffer, size_t length);
    std::string base64Decode(const std::string& input);
    std::string urlSafeBase64Encode(const std::string& input);
    std::string urlSafeBase64Decode(const std::string& input);
    std::string hmacSha256(const std::string& data, const std::string& key);
    utils::json::JSON parsePayload(const std::string& token);
};

} // namespace auth

#endif // JWT_HPP
