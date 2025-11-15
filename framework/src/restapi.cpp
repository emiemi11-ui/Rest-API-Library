#include "restapi.hpp"

// Include existing infrastructure
#include "../../Rest-API/include/core/server.hpp"
#include "../../Rest-API/include/http/router.hpp"
#include "../../Rest-API/include/http/request.hpp"
#include "../../Rest-API/include/http/response.hpp"

#include <iostream>
#include <sstream>
#include <algorithm>

namespace RestAPI {

// ===== HELPER FUNCTIONS =====

// Parse query parameters from target (/path?key=val&...)
static std::map<std::string, std::string> parseQuery(const std::string& target) {
    std::map<std::string, std::string> out;
    auto qpos = target.find('?');
    if (qpos == std::string::npos) return out;

    auto q = target.substr(qpos + 1);
    size_t i = 0;
    while (i < q.size()) {
        size_t amp = q.find('&', i);
        std::string pair = (amp == std::string::npos) ? q.substr(i) : q.substr(i, amp - i);
        size_t eq = pair.find('=');
        std::string k = (eq == std::string::npos) ? pair : pair.substr(0, eq);
        std::string v = (eq == std::string::npos) ? "" : pair.substr(eq + 1);
        out[k] = v;
        if (amp == std::string::npos) break;
        i = amp + 1;
    }
    return out;
}

// Convert HttpRequest to RestAPI::Request
static Request convertRequest(const HttpRequest& httpReq,
                               const std::map<std::string, std::string>& pathParams) {
    Request req;
    req.method = httpReq.method;
    req.path = httpReq.path;
    req.target = httpReq.target;
    req.body = httpReq.body;
    req.headers = httpReq.headers;
    req.params = pathParams;
    req.query = parseQuery(httpReq.target);
    req.raw = httpReq.raw;
    return req;
}

// Convert RestAPI::Response to HTTP response string
static std::string convertResponse(const Response& response) {
    std::ostringstream oss;

    // Status line
    oss << "HTTP/1.1 " << response.status << " ";
    switch (response.status) {
        case 200: oss << "OK"; break;
        case 201: oss << "Created"; break;
        case 204: oss << "No Content"; break;
        case 400: oss << "Bad Request"; break;
        case 401: oss << "Unauthorized"; break;
        case 403: oss << "Forbidden"; break;
        case 404: oss << "Not Found"; break;
        case 500: oss << "Internal Server Error"; break;
        default: oss << "Unknown"; break;
    }
    oss << "\r\n";

    // Headers
    for (const auto& [key, value] : response.headers) {
        oss << key << ": " << value << "\r\n";
    }

    // Content-Length
    oss << "Content-Length: " << response.body.size() << "\r\n";

    // End of headers
    oss << "\r\n";

    // Body
    oss << response.body;

    return oss.str();
}

// ===== IMPLEMENTATION CLASS =====
class RestApiFrameworkImpl {
public:
    int port;
    int workers;
    int thread_pool_size;
    bool cors_enabled;
    std::string cors_origins;
    std::string log_file;
    int log_level;
    int shutdown_timeout;

    Router router;
    std::unique_ptr<Server> server;

    std::vector<MiddlewareHandler> middlewares;

    RestApiFrameworkImpl(int p, int w)
        : port(p)
        , workers(w)
        , thread_pool_size(8)
        , cors_enabled(false)
        , cors_origins("*")
        , log_level(2)
        , shutdown_timeout(30)
    {}

    void registerRoute(const std::string& method, const std::string& path, RouteHandler handler) {
        // Wrap the RestAPI::RouteHandler into a function compatible with Router
        auto wrappedHandler = [handler, this](const HttpRequest& httpReq,
                                                const std::map<std::string, std::string>& params) -> std::string {
            // Convert to RestAPI::Request
            Request req = convertRequest(httpReq, params);

            // Execute middlewares
            Response res;
            for (auto& middleware : middlewares) {
                if (!middleware(req, res)) {
                    // Middleware rejected the request
                    return convertResponse(res);
                }
            }

            // CORS handling
            if (cors_enabled) {
                res.setHeader("Access-Control-Allow-Origin", cors_origins);
                res.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
                res.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
            }

            // Execute the handler
            Response response = handler(req);

            // Merge CORS headers if enabled
            if (cors_enabled) {
                response.setHeader("Access-Control-Allow-Origin", cors_origins);
                response.setHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
                response.setHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
            }

            // Convert and return
            return convertResponse(response);
        };

        // Register with the underlying Router
        router.addRoute(method, path, wrappedHandler);
    }
};

// ===== FRAMEWORK IMPLEMENTATION =====

RestApiFramework::RestApiFramework(int port, int workers)
    : pImpl(std::make_unique<RestApiFrameworkImpl>(port, workers))
{
}

RestApiFramework::~RestApiFramework() {
    if (pImpl && pImpl->server) {
        pImpl->server->stop();
    }
}

void RestApiFramework::get(const std::string& path, RouteHandler handler) {
    pImpl->registerRoute("GET", path, handler);
}

void RestApiFramework::post(const std::string& path, RouteHandler handler) {
    pImpl->registerRoute("POST", path, handler);
}

void RestApiFramework::put(const std::string& path, RouteHandler handler) {
    pImpl->registerRoute("PUT", path, handler);
}

void RestApiFramework::del(const std::string& path, RouteHandler handler) {
    pImpl->registerRoute("DELETE", path, handler);
}

void RestApiFramework::use(MiddlewareHandler middleware) {
    pImpl->middlewares.push_back(middleware);
}

void RestApiFramework::serve_static(const std::string& route, const std::string& directory) {
    // TODO: Implement static file serving
    (void)route;
    (void)directory;
    std::cerr << "[WARNING] Static file serving not yet implemented\n";
}

void RestApiFramework::start() {
    std::cout << "╔════════════════════════════════════════════════╗\n";
    std::cout << "║     REST API FRAMEWORK - C++                   ║\n";
    std::cout << "╚════════════════════════════════════════════════╝\n\n";

    std::cout << "[FRAMEWORK] Starting server...\n";
    std::cout << "  Port:    " << pImpl->port << "\n";
    std::cout << "  Workers: " << pImpl->workers << "\n";
    std::cout << "  CORS:    " << (pImpl->cors_enabled ? "enabled" : "disabled") << "\n\n";

    // Create server instance
    pImpl->server = std::make_unique<Server>(pImpl->port, pImpl->workers);
    pImpl->server->setRouter(pImpl->router);

    std::cout << "Server listening on http://localhost:" << pImpl->port << "\n\n";

    // Start server (blocking)
    pImpl->server->start();
}

void RestApiFramework::stop() {
    if (pImpl && pImpl->server) {
        pImpl->server->stop();
    }
}

void RestApiFramework::shutdown() {
    if (pImpl && pImpl->server) {
        pImpl->server->request_shutdown();
    }
}

void RestApiFramework::set_workers(int count) {
    pImpl->workers = count;
}

void RestApiFramework::set_thread_pool_size(int size) {
    pImpl->thread_pool_size = size;
}

void RestApiFramework::enable_cors(bool enable) {
    pImpl->cors_enabled = enable;
}

void RestApiFramework::set_cors_origins(const std::string& origins) {
    pImpl->cors_origins = origins;
}

void RestApiFramework::enable_logging(const std::string& log_file) {
    pImpl->log_file = log_file;
}

void RestApiFramework::set_log_level(int level) {
    pImpl->log_level = level;
}

void RestApiFramework::set_shutdown_timeout(int seconds) {
    pImpl->shutdown_timeout = seconds;
}

int RestApiFramework::get_port() const {
    return pImpl->port;
}

int RestApiFramework::get_workers() const {
    return pImpl->workers;
}

} // namespace RestAPI
