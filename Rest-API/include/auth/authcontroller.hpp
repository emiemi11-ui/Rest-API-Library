#ifndef AUTHCONTROLLER_HPP
#define AUTHCONTROLLER_HPP

#include <memory>
#include <string>

// Forward declarations
namespace http {
    class Request;
    class Response;
}

namespace services {
    class UserService;
}

namespace auth {
    class JWT;
}

namespace auth {

/**
 * @brief Authentication controller for login/register endpoints
 *
 * Handles user authentication and registration.
 */
class AuthController {
public:
    /**
     * @brief Constructor
     * @param jwt JWT instance for token generation
     * @param user_service User service for user management
     */
    AuthController(std::shared_ptr<JWT> jwt, std::shared_ptr<services::UserService> user_service);

    /**
     * @brief Handle login request
     * @param request HTTP request with email and password
     * @param response HTTP response with JWT token
     */
    void login(const http::Request& request, http::Response& response);

    /**
     * @brief Handle register request
     * @param request HTTP request with user details
     * @param response HTTP response with success message
     */
    void registerUser(const http::Request& request, http::Response& response);

    /**
     * @brief Handle logout request
     * @param request HTTP request
     * @param response HTTP response
     */
    void logout(const http::Request& request, http::Response& response);

    /**
     * @brief Get current user profile
     * @param request HTTP request with valid JWT
     * @param response HTTP response with user data
     */
    void getProfile(const http::Request& request, http::Response& response);

    /**
     * @brief Refresh JWT token
     * @param request HTTP request with valid JWT
     * @param response HTTP response with new JWT token
     */
    void refreshToken(const http::Request& request, http::Response& response);

private:
    std::shared_ptr<JWT> jwt_;
    std::shared_ptr<services::UserService> user_service_;

    bool validateLoginRequest(const http::Request& request, std::string& email, std::string& password);
    bool validateRegisterRequest(const http::Request& request);
};

} // namespace auth

#endif // AUTHCONTROLLER_HPP
