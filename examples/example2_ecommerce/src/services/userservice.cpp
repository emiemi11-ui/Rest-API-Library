#include "services/userservice.hpp"
#include <iostream>
#include <stdexcept>
#include <regex>
#include <iomanip>
#include <sstream>
#include <openssl/sha.h>

std::vector<User> UserService::getAllUsers() {
    std::cout << "[UserService] getAllUsers()\n";
    return repository.findAll();
}

std::optional<User> UserService::getUserById(int id) {
    std::cout << "[UserService] getUserById(" << id << ")\n";
    
    if (id <= 0) {
        throw std::invalid_argument("ID invalid: trebuie să fie pozitiv");
    }
    
    return repository.findById(id);
}

User UserService::createUser(const std::string& name, const std::string& email) {
    std::cout << "[UserService] createUser(" << name << ", " << email << ")\n";
    
    // Validări
    if (name.empty()) {
        throw std::invalid_argument("Numele nu poate fi gol");
    }
    if (name.length() > 255) {
        throw std::invalid_argument("Numele este prea lung (max 255 caractere)");
    }
    
    validateEmail(email);
    
    // Verifică dacă email-ul există deja
    if (repository.existsByEmail(email)) {
        throw std::invalid_argument("Email-ul există deja: " + email);
    }
    
    // Creează user-ul
    User user;
    user.name = name;
    user.email = email;
    
    return repository.save(user);
}

void UserService::updateUser(int id, const User& user) {
    std::cout << "[UserService] updateUser(" << id << ")\n";
    
    validateUserForUpdate(id);
    validateUser(user);
    
    // Verifică dacă email-ul este folosit de alt user
    auto existing = repository.findByEmail(user.email);
    if (existing.has_value() && existing->id != id) {
        throw std::invalid_argument("Email-ul este folosit de alt utilizator");
    }
    
    User updated_user = user;
    updated_user.id = id;
    
    repository.update(updated_user);
}

void UserService::deleteUser(int id) {
    std::cout << "[UserService] deleteUser(" << id << ")\n";
    
    validateUserForUpdate(id);
    repository.deleteById(id);
}

void UserService::validateEmail(const std::string& email) {
    if (email.empty()) {
        throw std::invalid_argument("Email-ul nu poate fi gol");
    }
    
    if (email.length() > 255) {
        throw std::invalid_argument("Email-ul este prea lung (max 255 caractere)");
    }
    
    // Validare simplă de email
    std::regex email_regex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    if (!std::regex_match(email, email_regex)) {
        throw std::invalid_argument("Format email invalid: " + email);
    }
}

void UserService::validateUser(const User& user) {
    if (user.name.empty()) {
        throw std::invalid_argument("Numele nu poate fi gol");
    }
    if (user.name.length() > 255) {
        throw std::invalid_argument("Numele este prea lung (max 255 caractere)");
    }
    validateEmail(user.email);
}

void UserService::validateUserForUpdate(int id) {
    if (id <= 0) {
        throw std::invalid_argument("ID invalid: trebuie să fie pozitiv");
    }

    auto user = repository.findById(id);
    if (!user.has_value()) {
        throw std::invalid_argument("Utilizatorul cu ID=" + std::to_string(id) + " nu există");
    }
}

// ========== AUTENTIFICARE ==========

std::string UserService::hashPassword(const std::string& password) {
    // NOTA: In productie foloseste bcrypt sau argon2!
    // Aici folosim SHA256 pentru simplitate
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(password.c_str()),
           password.length(), hash);

    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(hash[i]);
    }
    return oss.str();
}

bool UserService::verifyPassword(const std::string& password, const std::string& hash) {
    return hashPassword(password) == hash;
}

void UserService::validatePassword(const std::string& password) {
    if (password.empty()) {
        throw std::invalid_argument("Parola nu poate fi goală");
    }
    if (password.length() < 6) {
        throw std::invalid_argument("Parola trebuie să aibă cel puțin 6 caractere");
    }
    if (password.length() > 255) {
        throw std::invalid_argument("Parola este prea lungă (max 255 caractere)");
    }
}

User UserService::registerUser(const std::string& name, const std::string& email,
                               const std::string& password) {
    std::cout << "[UserService] registerUser(" << name << ", " << email << ")\n";

    // Validări
    if (name.empty()) {
        throw std::invalid_argument("Numele nu poate fi gol");
    }
    if (name.length() > 255) {
        throw std::invalid_argument("Numele este prea lung (max 255 caractere)");
    }

    validateEmail(email);
    validatePassword(password);

    // Verifică dacă email-ul există deja
    if (repository.existsByEmail(email)) {
        throw std::invalid_argument("Email-ul există deja: " + email);
    }

    // Creează user-ul cu parola hash-uită
    User user;
    user.name = name;
    user.email = email;
    user.password_hash = hashPassword(password);

    return repository.save(user);
}

std::optional<User> UserService::loginUser(const std::string& email,
                                           const std::string& password) {
    std::cout << "[UserService] loginUser(" << email << ")\n";

    validateEmail(email);

    if (password.empty()) {
        throw std::invalid_argument("Parola nu poate fi goală");
    }

    // Caută user-ul după email
    auto user = repository.findByEmail(email);
    if (!user.has_value()) {
        std::cout << "[UserService] User nu există: " << email << "\n";
        return std::nullopt;
    }

    // Verifică parola
    if (!verifyPassword(password, user->password_hash)) {
        std::cout << "[UserService] Parolă incorectă pentru: " << email << "\n";
        return std::nullopt;
    }

    std::cout << "[UserService] Login success: " << email << "\n";
    return user;
}
