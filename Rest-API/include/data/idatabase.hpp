#pragma once
#include <string>
#include <vector>
#include <map>

/// Interfață generică DB (portabilă: SQLite/MySQL/Postgres etc.)
class IDatabase {
public:
    virtual ~IDatabase() = default;
    virtual bool connect(const std::map<std::string,std::string>& cfg) = 0;
    virtual void disconnect() = 0;
    virtual bool execute(const std::string& sql) = 0;  // INSERT/UPDATE/DDL
    virtual std::vector<std::map<std::string,std::string>> query(const std::string& sql) = 0; // SELECT
};
