#pragma once
#include <string>
#include <map>

class HttpRequest {
public:
    // Linia de start
    std::string method;   // "GET", "POST", ...
    std::string target;   // EX: "/api/users/add?name=Ana"  (!!! NOU)
    std::string path;     // EX: "/api/users/add"           (!!! util pt rutare)

    // Headere + body
    std::map<std::string, std::string> headers;
    std::string body;

    // Request raw complet (pentru parsing manual in controller)
    std::string raw;

    // utilitare simple (nu rup nimic existent)
    const std::string& getMethod() const { return method; }
    const std::string& getTarget() const { return target; }
    const std::string& getPath()   const { return path;   }
};
