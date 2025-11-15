#pragma once

#include <functional>
#include <string>
#include <map>
#include <memory>

namespace RestAPI {

// Forward declarations
class Request;
class Response;
class RestApiFrameworkImpl;

// ===== REQUEST CLASS =====
class Request {
public:
    std::string method;         // GET, POST, PUT, DELETE
    std::string path;           // /api/resource
    std::string target;         // /api/resource?key=value
    std::string body;           // Request body

    std::map<std::string, std::string> params;   // Path parameters (:id)
    std::map<std::string, std::string> query;    // Query parameters (?key=value)
    std::map<std::string, std::string> headers;  // HTTP headers

    std::string raw;            // Raw request (for advanced use)

    // Helper methods
    const std::string& getMethod() const { return method; }
    const std::string& getPath() const { return path; }
    const std::string& getBody() const { return body; }

    // Get header value
    std::string getHeader(const std::string& key) const {
        auto it = headers.find(key);
        return (it != headers.end()) ? it->second : "";
    }

    // Get query parameter
    std::string getQuery(const std::string& key) const {
        auto it = query.find(key);
        return (it != query.end()) ? it->second : "";
    }

    // Get path parameter
    std::string getParam(const std::string& key) const {
        auto it = params.find(key);
        return (it != params.end()) ? it->second : "";
    }
};

// ===== RESPONSE CLASS =====
class Response {
public:
    int status;
    std::string body;
    std::map<std::string, std::string> headers;

    Response() : status(200) {}
    Response(int s, const std::string& b) : status(s), body(b) {}

    // Static factory methods
    static Response json(int status, const std::string& data) {
        Response r(status, data);
        r.headers["Content-Type"] = "application/json";
        return r;
    }

    static Response text(int status, const std::string& data) {
        Response r(status, data);
        r.headers["Content-Type"] = "text/plain";
        return r;
    }

    static Response html(int status, const std::string& data) {
        Response r(status, data);
        r.headers["Content-Type"] = "text/html";
        return r;
    }

    // Set custom header
    Response& setHeader(const std::string& key, const std::string& value) {
        headers[key] = value;
        return *this;
    }
};

// Type aliases for handler functions
using RouteHandler = std::function<Response(const Request&)>;
using MiddlewareHandler = std::function<bool(Request&, Response&)>;

// ===== MAIN FRAMEWORK CLASS =====
class RestApiFramework {
public:
    // Constructor
    explicit RestApiFramework(int port = 8080, int workers = 4);

    // Destructor
    ~RestApiFramework();

    // Delete copy constructor and assignment operator (non-copyable)
    RestApiFramework(const RestApiFramework&) = delete;
    RestApiFramework& operator=(const RestApiFramework&) = delete;

    // ===== ROUTE REGISTRATION =====

    // Register GET route
    void get(const std::string& path, RouteHandler handler);

    // Register POST route
    void post(const std::string& path, RouteHandler handler);

    // Register PUT route
    void put(const std::string& path, RouteHandler handler);

    // Register DELETE route
    void del(const std::string& path, RouteHandler handler);

    // ===== MIDDLEWARE =====

    // Add middleware (executed before route handlers)
    void use(MiddlewareHandler middleware);

    // ===== STATIC FILES =====

    // Serve static files from a directory
    void serve_static(const std::string& route, const std::string& directory);

    // ===== SERVER CONTROL =====

    // Start the server (blocking call)
    void start();

    // Stop the server
    void stop();

    // Request graceful shutdown
    void shutdown();

    // ===== CONFIGURATION =====

    // Set number of worker processes
    void set_workers(int count);

    // Set thread pool size per worker
    void set_thread_pool_size(int size);

    // Enable/disable CORS
    void enable_cors(bool enable = true);

    // Set CORS allowed origins
    void set_cors_origins(const std::string& origins);

    // Enable logging to file
    void enable_logging(const std::string& log_file = "server.log");

    // Set log level (0=ERROR, 1=WARN, 2=INFO, 3=DEBUG)
    void set_log_level(int level);

    // Set shutdown timeout
    void set_shutdown_timeout(int seconds);

    // Get server port
    int get_port() const;

    // Get number of workers
    int get_workers() const;

private:
    // pImpl pattern - hide implementation details
    std::unique_ptr<RestApiFrameworkImpl> pImpl;
};

} // namespace RestAPI
