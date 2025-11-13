#ifndef CORS_HPP
#define CORS_HPP

#include <string>
#include <vector>

// Forward declarations
namespace http {
    class Request;
    class Response;
}

namespace middleware {

/**
 * @brief CORS (Cross-Origin Resource Sharing) middleware
 *
 * Handles CORS headers and preflight requests.
 */
class CORS {
public:
    /**
     * @brief Constructor
     * @param allowed_origins Allowed origins (use "*" for all)
     * @param allowed_methods Allowed HTTP methods
     * @param allowed_headers Allowed request headers
     * @param max_age Max age for preflight cache (seconds)
     */
    CORS(const std::string& allowed_origins = "*",
         const std::vector<std::string>& allowed_methods = {"GET", "POST", "PUT", "DELETE", "OPTIONS"},
         const std::vector<std::string>& allowed_headers = {"Content-Type", "Authorization"},
         int max_age = 86400);

    /**
     * @brief Add CORS headers to response
     * @param request HTTP request
     * @param response HTTP response
     */
    void addHeaders(const http::Request& request, http::Response& response);

    /**
     * @brief Handle preflight OPTIONS request
     * @param request HTTP request
     * @param response HTTP response
     * @return true if this was a preflight request, false otherwise
     */
    bool handlePreflight(const http::Request& request, http::Response& response);

private:
    std::string allowed_origins_;
    std::vector<std::string> allowed_methods_;
    std::vector<std::string> allowed_headers_;
    int max_age_;

    std::string joinVector(const std::vector<std::string>& vec, const std::string& delimiter);
};

} // namespace middleware

#endif // CORS_HPP
