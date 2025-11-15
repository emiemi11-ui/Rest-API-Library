#include "http/response.hpp"
#include <sstream>

static const char* status_text(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 500: return "Internal Server Error";
        default:  return "Unknown";
    }
}

std::string HttpResponse::json(int status, const std::string& body) {
    std::ostringstream res;
    res << "HTTP/1.1 " << status << " " << status_text(status) << "\r\n";
    res << "Content-Type: application/json\r\n";
    res << "Content-Length: " << body.size() << "\r\n";
    res << "Connection: close\r\n";
    res << "\r\n";
    res << body;
    return res.str();
}
