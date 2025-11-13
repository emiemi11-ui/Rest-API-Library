#ifndef AUTHMIDDLEWARE_HPP
#define AUTHMIDDLEWARE_HPP

#include "../http/request.hpp"
#include "../http/response.hpp"
#include "../auth/jwt.hpp"
#include <string>
#include <vector>
#include <memory>

namespace middleware {

/**
 * @brief Authentication middleware for protecting API endpoints
 *
 * Validates JWT tokens and enforces role-based access control.
 */
class AuthMiddleware {
public:
    /**
     * @brief Constructor
     * @param jwt JWT instance for token validation
     */
    explicit AuthMiddleware(std::shared_ptr<auth::JWT> jwt);

    /**
     * @brief Check if request has valid authentication
     * @param request HTTP request
     * @param response HTTP response (used to send error if authentication fails)
     * @return true if authenticated, false otherwise
     */
    bool authenticate(const http::Request& request, http::Response& response);

    /**
     * @brief Check if user has required role
     * @param request HTTP request
     * @param required_roles List of acceptable roles
     * @return true if user has one of the required roles
     */
    bool authorize(const http::Request& request, const std::vector<std::string>& required_roles);

    /**
     * @brief Extract user ID from request
     * @param request HTTP request
     * @return User ID, or -1 if not authenticated
     */
    int getUserId(const http::Request& request);

    /**
     * @brief Extract username from request
     * @param request HTTP request
     * @return Username, or empty string if not authenticated
     */
    std::string getUsername(const http::Request& request);

    /**
     * @brief Extract role from request
     * @param request HTTP request
     * @return Role, or empty string if not authenticated
     */
    std::string getRole(const http::Request& request);

private:
    std::shared_ptr<auth::JWT> jwt_;

    std::string extractToken(const http::Request& request);
};

} // namespace middleware

#endif // AUTHMIDDLEWARE_HPP
