#pragma once
#include "data/idatabase.hpp"
#include <sqlite3.h>

class SqliteDatabase : public IDatabase {
    sqlite3* db {nullptr};
public:
    ~SqliteDatabase() override { disconnect(); }
    bool connect(const std::map<std::string,std::string>& cfg) override;
    void disconnect() override;
    bool execute(const std::string& sql) override;
    std::vector<std::map<std::string,std::string>> query(const std::string& sql) override;
};
