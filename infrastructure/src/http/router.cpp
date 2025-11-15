#include "http/router.hpp"
#include <iostream>
#include <sstream>

void Router::addRoute(const std::string& method, const std::string& pattern, RouteHandler handler) {
    routes.push_back({method, pattern, handler});
    std::cout << "[Router] Rută adăugată: " << method << " " << pattern << "\n";
}

std::string Router::handle(const HttpRequest& request) {
    std::cout << "[Router] Procesare: " << request.method << " " << request.path << "\n";
    
    // Caută o rută potrivită
    for (const auto& route : routes) {
        if (route.method == request.method) {
            std::map<std::string, std::string> params;
            if (matchPattern(route.pattern, request.path, params)) {
                std::cout << "[Router] Match găsit: " << route.pattern << "\n";
                try {
                    return route.handler(request, params);
                } catch (const std::exception& e) {
                    std::cerr << "[Router] Eroare în handler: " << e.what() << "\n";
                    // Returnează răspuns de eroare
                    std::ostringstream response;
                    response << "HTTP/1.1 500 Internal Server Error\r\n";
                    response << "Content-Type: application/json\r\n";
                    response << "Connection: close\r\n\r\n";
                    response << "{\"error\":\"" << e.what() << "\"}";
                    return response.str();
                }
            }
        }
    }
    
    // Nicio rută nu a fost găsită
    std::cout << "[Router] Nicio rută găsită pentru " << request.method << " " << request.path << "\n";
    std::ostringstream response;
    response << "HTTP/1.1 404 Not Found\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Connection: close\r\n\r\n";
    response << "{\"error\":\"Not Found\",\"path\":\"" << request.path << "\"}";
    return response.str();
}

bool Router::matchPattern(const std::string& pattern, const std::string& path,
                         std::map<std::string, std::string>& params) {
    std::vector<std::string> pattern_parts = splitPath(pattern);
    std::vector<std::string> path_parts = splitPath(path);
    
    if (pattern_parts.size() != path_parts.size()) {
        return false;
    }
    
    for (size_t i = 0; i < pattern_parts.size(); ++i) {
        if (pattern_parts[i][0] == ':') {
            // Parametru dinamic (ex: :id)
            std::string param_name = pattern_parts[i].substr(1);
            params[param_name] = path_parts[i];
        } else if (pattern_parts[i] != path_parts[i]) {
            // Segmentul nu match
            return false;
        }
    }
    
    return true;
}

std::vector<std::string> Router::splitPath(const std::string& path) {
    std::vector<std::string> parts;
    std::string current;
    
    for (char c : path) {
        if (c == '/') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    
    if (!current.empty()) {
        parts.push_back(current);
    }
    
    return parts;
}
