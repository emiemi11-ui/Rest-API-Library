#include "data/userrepository.hpp"
#include <sstream>
#include <iostream>

// Helper pentru escape SQL (simplu - pentru productie foloseste prepared statements)
static std::string escapeSql(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (c == '\'') result += "''";
        else result += c;
    }
    return result;
}

bool UserRepository::init() {
    // Creeaza tabelul cu schema completa pentru autentificare
    std::string sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "email TEXT NOT NULL UNIQUE, "
        "password_hash TEXT NOT NULL"
        ")";
    return conn.db().execute(sql);
}

std::vector<User> UserRepository::findAll() {
    std::vector<User> v;
    std::string sql = "SELECT id, name, email, password_hash FROM users ORDER BY id ASC";
    for (auto& row : conn.db().query(sql)) {
        User u{};
        if (row.count("id"))            u.id = std::stoi(row.at("id"));
        if (row.count("name"))          u.name = row.at("name");
        if (row.count("email"))         u.email = row.at("email");
        if (row.count("password_hash")) u.password_hash = row.at("password_hash");
        v.push_back(std::move(u));
    }
    return v;
}

std::optional<User> UserRepository::findById(int id) {
    std::ostringstream sql;
    sql << "SELECT id, name, email, password_hash FROM users WHERE id = " << id;

    auto rows = conn.db().query(sql.str());
    if (rows.empty()) {
        return std::nullopt;
    }

    auto& row = rows[0];
    User u{};
    if (row.count("id"))            u.id = std::stoi(row.at("id"));
    if (row.count("name"))          u.name = row.at("name");
    if (row.count("email"))         u.email = row.at("email");
    if (row.count("password_hash")) u.password_hash = row.at("password_hash");

    return u;
}

std::optional<User> UserRepository::findByEmail(const std::string& email) {
    std::ostringstream sql;
    sql << "SELECT id, name, email, password_hash FROM users WHERE email = '"
        << escapeSql(email) << "'";

    auto rows = conn.db().query(sql.str());
    if (rows.empty()) {
        return std::nullopt;
    }

    auto& row = rows[0];
    User u{};
    if (row.count("id"))            u.id = std::stoi(row.at("id"));
    if (row.count("name"))          u.name = row.at("name");
    if (row.count("email"))         u.email = row.at("email");
    if (row.count("password_hash")) u.password_hash = row.at("password_hash");

    return u;
}

bool UserRepository::existsByEmail(const std::string& email) {
    std::ostringstream sql;
    sql << "SELECT COUNT(*) as cnt FROM users WHERE email = '"
        << escapeSql(email) << "'";

    auto rows = conn.db().query(sql.str());
    if (rows.empty()) return false;

    int count = std::stoi(rows[0].at("cnt"));
    return count > 0;
}

User UserRepository::save(const User& user) {
    std::ostringstream sql;
    sql << "INSERT INTO users(name, email, password_hash) VALUES('"
        << escapeSql(user.name) << "', '"
        << escapeSql(user.email) << "', '"
        << escapeSql(user.password_hash) << "')";

    if (!conn.db().execute(sql.str())) {
        throw std::runtime_error("Failed to insert user");
    }

    // Obtine ID-ul generat
    auto rows = conn.db().query("SELECT last_insert_rowid() as id");
    if (rows.empty()) {
        throw std::runtime_error("Failed to get inserted user ID");
    }

    User created = user;
    created.id = std::stoi(rows[0].at("id"));
    return created;
}

void UserRepository::update(const User& user) {
    std::ostringstream sql;
    sql << "UPDATE users SET "
        << "name = '" << escapeSql(user.name) << "', "
        << "email = '" << escapeSql(user.email) << "'";

    // Update password doar daca e setat
    if (!user.password_hash.empty()) {
        sql << ", password_hash = '" << escapeSql(user.password_hash) << "'";
    }

    sql << " WHERE id = " << user.id;

    if (!conn.db().execute(sql.str())) {
        throw std::runtime_error("Failed to update user");
    }
}

void UserRepository::deleteById(int id) {
    std::ostringstream sql;
    sql << "DELETE FROM users WHERE id = " << id;

    if (!conn.db().execute(sql.str())) {
        throw std::runtime_error("Failed to delete user");
    }
}

// Legacy method pentru compatibilitate cu main.cpp vechi
bool UserRepository::add(const std::string& name) {
    // Pentru compatibilitate - creeaza user cu email dummy
    std::ostringstream sql;
    std::string email = name + "@legacy.com";
    sql << "INSERT INTO users(name, email, password_hash) VALUES('"
        << escapeSql(name) << "', '"
        << escapeSql(email) << "', 'legacy')";
    return conn.db().execute(sql.str());
}
