#include "controllers/usercontroller.hpp"
#include <iostream>
#include <sstream>

void UserController::setRawRequest(const std::string& raw) {
    raw_request = raw;
}

std::string UserController::extractBody(const std::string& raw_req) {
    // Găsește body-ul după \r\n\r\n
    size_t pos = raw_req.find("\r\n\r\n");
    if (pos == std::string::npos) {
        return "";
    }
    return raw_req.substr(pos + 4);
}

std::string UserController::jsonResponse(int status, const std::string& body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << status;

    switch (status) {
        case 200: response << " OK"; break;
        case 201: response << " Created"; break;
        case 204: response << " No Content"; break;
        case 400: response << " Bad Request"; break;
        case 401: response << " Unauthorized"; break;
        case 404: response << " Not Found"; break;
        case 500: response << " Internal Server Error"; break;
        default: response << " Unknown"; break;
    }

    response << "\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}

std::string UserController::getAll(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[UserController] GET /api/users\n";
    
    try {
        auto users = service.getAllUsers();
        
        // Construiește JSON array
        std::ostringstream json;
        json << "[";
        for (size_t i = 0; i < users.size(); ++i) {
            json << users[i].toJson();
            if (i < users.size() - 1) {
                json << ",";
            }
        }
        json << "]";
        
        return jsonResponse(200, json.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

std::string UserController::getById(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[UserController] GET /api/users/:id\n";
    
    try {
        // Extrage ID din parametri
        auto it = params.find("id");
        if (it == params.end()) {
            return jsonResponse(400, "{\"error\":\"ID lipsă\"}");
        }
        
        int id = std::stoi(it->second);
        auto user = service.getUserById(id);
        
        if (!user.has_value()) {
            std::ostringstream error;
            error << "{\"error\":\"User cu ID=" << id << " nu a fost găsit\"}";
            return jsonResponse(404, error.str());
        }
        
        return jsonResponse(200, user->toJson());
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

std::string UserController::create(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[UserController] POST /api/users\n";
    
    try {
        // Extrage body-ul
        std::string body = extractBody(raw_request);
        std::cout << "[UserController] Body: " << body << "\n";
        
        if (body.empty()) {
            return jsonResponse(400, "{\"error\":\"Body lipsă\"}");
        }
        
        // Parsează JSON (simplu)
        User user = User::fromJson(body);
        
        // Creează user-ul
        User created = service.createUser(user.name, user.email);
        
        return jsonResponse(201, created.toJson());
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

std::string UserController::update(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[UserController] PUT /api/users/:id\n";
    
    try {
        // Extrage ID
        auto it = params.find("id");
        if (it == params.end()) {
            return jsonResponse(400, "{\"error\":\"ID lipsă\"}");
        }
        int id = std::stoi(it->second);
        
        // Extrage body
        std::string body = extractBody(raw_request);
        if (body.empty()) {
            return jsonResponse(400, "{\"error\":\"Body lipsă\"}");
        }
        
        // Parsează JSON
        User user = User::fromJson(body);
        
        // Update
        service.updateUser(id, user);
        
        // Obține user-ul actualizat
        auto updated = service.getUserById(id);
        if (updated.has_value()) {
            return jsonResponse(200, updated->toJson());
        } else {
            return jsonResponse(200, "{\"message\":\"User actualizat cu succes\"}");
        }
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

std::string UserController::remove(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[UserController] DELETE /api/users/:id\n";

    try {
        // Extrage ID
        auto it = params.find("id");
        if (it == params.end()) {
            return jsonResponse(400, "{\"error\":\"ID lipsă\"}");
        }
        int id = std::stoi(it->second);

        // Șterge
        service.deleteUser(id);

        return jsonResponse(200, "{\"message\":\"User șters cu succes\"}");
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

// ========== AUTENTIFICARE ==========

std::string UserController::registerUser(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[UserController] POST /api/auth/register\n";

    try {
        // Extrage body-ul
        std::string body = extractBody(raw_request);
        std::cout << "[UserController] Body: " << body << "\n";

        if (body.empty()) {
            return jsonResponse(400, "{\"error\":\"Body lipsă\"}");
        }

        // Parsează JSON pentru a extrage name, email și password
        User user = User::fromJson(body);

        // Extrage password din JSON (manual, simplu)
        std::string password;
        std::string search = "\"password\":\"";
        size_t pos = body.find(search);
        if (pos != std::string::npos) {
            pos += search.length();
            size_t end = body.find("\"", pos);
            if (end != std::string::npos) {
                password = body.substr(pos, end - pos);
            }
        }

        if (password.empty()) {
            return jsonResponse(400, "{\"error\":\"Parola lipsă\"}");
        }

        // Înregistrează user-ul
        User created = service.registerUser(user.name, user.email, password);

        std::ostringstream response;
        response << "{\"message\":\"Înregistrare cu succes\",\"user\":" << created.toJson() << "}";

        return jsonResponse(201, response.str());
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

std::string UserController::loginUser(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[UserController] POST /api/auth/login\n";

    try {
        // Extrage body-ul
        std::string body = extractBody(raw_request);
        std::cout << "[UserController] Body: " << body << "\n";

        if (body.empty()) {
            return jsonResponse(400, "{\"error\":\"Body lipsă\"}");
        }

        // Parsează email și password din JSON
        std::string email, password;

        // Extrage email
        std::string email_search = "\"email\":\"";
        size_t email_pos = body.find(email_search);
        if (email_pos != std::string::npos) {
            email_pos += email_search.length();
            size_t email_end = body.find("\"", email_pos);
            if (email_end != std::string::npos) {
                email = body.substr(email_pos, email_end - email_pos);
            }
        }

        // Extrage password
        std::string pass_search = "\"password\":\"";
        size_t pass_pos = body.find(pass_search);
        if (pass_pos != std::string::npos) {
            pass_pos += pass_search.length();
            size_t pass_end = body.find("\"", pass_pos);
            if (pass_end != std::string::npos) {
                password = body.substr(pass_pos, pass_end - pass_pos);
            }
        }

        if (email.empty() || password.empty()) {
            return jsonResponse(400, "{\"error\":\"Email și parola sunt obligatorii\"}");
        }

        // Autentifică user-ul
        auto user = service.loginUser(email, password);

        if (!user.has_value()) {
            return jsonResponse(401, "{\"error\":\"Email sau parolă incorectă\"}");
        }

        std::ostringstream response;
        response << "{\"message\":\"Login cu succes\",\"user\":" << user->toJson() << "}";

        return jsonResponse(200, response.str());
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}
