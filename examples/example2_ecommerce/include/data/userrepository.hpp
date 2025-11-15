#pragma once
#include "data/databaseconnection.hpp"
#include "models/user.hpp"
#include <string>
#include <vector>
#include <optional>

class UserRepository {
    DatabaseConnection& conn;
public:
    explicit UserRepository(DatabaseConnection& c): conn(c) {}

    // Initializare tabela cu schema completa
    bool init();

    // CRUD operations
    std::vector<User> findAll();
    std::optional<User> findById(int id);
    std::optional<User> findByEmail(const std::string& email);
    bool existsByEmail(const std::string& email);
    User save(const User& user);
    void update(const User& user);
    void deleteById(int id);

    // Legacy methods pentru compatibilitate
    std::vector<User> all() { return findAll(); }
    bool add(const std::string& name);
};
