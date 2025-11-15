#pragma once
#include "services/userservice.hpp"
#include "http/request.hpp"
#include <string>
#include <map>

class UserController {
private:
    UserService& service;
    
    // Helper pentru a extrage body-ul din request
    std::string extractBody(const std::string& raw_request);
    
    // Helper pentru a construi un răspuns HTTP JSON
    std::string jsonResponse(int status, const std::string& body);

public:
    explicit UserController(UserService& service) : service(service) {}
    
    // GET /api/users - Obține toți utilizatorii
    std::string getAll(const HttpRequest& req, const std::map<std::string, std::string>& params);
    
    // GET /api/users/:id - Obține un utilizator specific
    std::string getById(const HttpRequest& req, const std::map<std::string, std::string>& params);
    
    // POST /api/users - Creează un utilizator nou
    std::string create(const HttpRequest& req, const std::map<std::string, std::string>& params);
    
    // PUT /api/users/:id - Actualizează un utilizator
    std::string update(const HttpRequest& req, const std::map<std::string, std::string>& params);
    
    // DELETE /api/users/:id - Șterge un utilizator
    std::string remove(const HttpRequest& req, const std::map<std::string, std::string>& params);

    // POST /api/auth/register - Înregistrare user nou
    std::string registerUser(const HttpRequest& req, const std::map<std::string, std::string>& params);

    // POST /api/auth/login - Autentificare user
    std::string loginUser(const HttpRequest& req, const std::map<std::string, std::string>& params);

    // Setează raw request pentru parsing body
    void setRawRequest(const std::string& raw);
    
private:
    std::string raw_request;
};
