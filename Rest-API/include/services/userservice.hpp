#pragma once
#include "data/userrepository.hpp"
#include "models/user.hpp"
#include <vector>
#include <string>
#include <optional>

class UserService {
private:
    UserRepository& repository;

    // Helper pentru hash password (simplu - in productie foloseste bcrypt/argon2)
    std::string hashPassword(const std::string& password);
    bool verifyPassword(const std::string& password, const std::string& hash);

public:
    explicit UserService(UserRepository& repository) : repository(repository) {}

    // Operații CRUD cu validări
    std::vector<User> getAllUsers();
    std::optional<User> getUserById(int id);
    User createUser(const std::string& name, const std::string& email);
    void updateUser(int id, const User& user);
    void deleteUser(int id);

    // Autentificare
    User registerUser(const std::string& name, const std::string& email, const std::string& password);
    std::optional<User> loginUser(const std::string& email, const std::string& password);

    // Validări business logic
    void validateEmail(const std::string& email);
    void validateUser(const User& user);
    void validateUserForUpdate(int id);
    void validatePassword(const std::string& password);
};
