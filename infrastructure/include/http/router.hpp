#pragma once
#include "http/request.hpp"
#include "http/response.hpp"
#include <functional>
#include <map>
#include <string>
#include <vector>

// Tip pentru handler functions
using RouteHandler = std::function<std::string(const HttpRequest&, const std::map<std::string, std::string>&)>;

struct Route {
    std::string method;
    std::string pattern;  // ex: "/api/users/:id"
    RouteHandler handler;
};

class Router {
private:
    std::vector<Route> routes;
    
    // Helper pentru a verifica dacă un pattern match cu un path
    bool matchPattern(const std::string& pattern, const std::string& path, 
                     std::map<std::string, std::string>& params);
    
    // Helper pentru a splita path-ul în segmente
    std::vector<std::string> splitPath(const std::string& path);

public:
    Router() = default;
    
    // Adaugă o rută
    void addRoute(const std::string& method, const std::string& pattern, RouteHandler handler);
    
    // Găsește și execută handler-ul pentru o cerere
    std::string handle(const HttpRequest& request);
    
    // Helper shortcuts pentru metode HTTP
    void get(const std::string& pattern, RouteHandler handler) {
        addRoute("GET", pattern, handler);
    }
    
    void post(const std::string& pattern, RouteHandler handler) {
        addRoute("POST", pattern, handler);
    }
    
    void put(const std::string& pattern, RouteHandler handler) {
        addRoute("PUT", pattern, handler);
    }
    
    void del(const std::string& pattern, RouteHandler handler) {
        addRoute("DELETE", pattern, handler);
    }
};
