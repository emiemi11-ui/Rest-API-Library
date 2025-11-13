#include "data/sqlitedatabase.hpp"
#include <iostream>

static int cb_rows(void* data, int argc, char** argv, char** colnames) {
    auto* rows = reinterpret_cast<std::vector<std::map<std::string,std::string>>*>(data);
    std::map<std::string,std::string> row;
    for (int i=0;i<argc;++i) {
        row[colnames[i] ? colnames[i] : ""] = argv[i] ? argv[i] : "";
    }
    rows->push_back(std::move(row));
    return 0;
}

bool SqliteDatabase::connect(const std::map<std::string,std::string>& cfg) {
    auto it = cfg.find("file");
    if (it == cfg.end()) return false;
    const std::string& path = it->second;
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "SQLite open error: " << (db ? sqlite3_errmsg(db) : "null") << "\n";
        return false;
    }
    return true;
}

void SqliteDatabase::disconnect() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}

bool SqliteDatabase::execute(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::cerr << "SQLite exec error: " << (err ? err : "") << "\n";
        if (err) sqlite3_free(err);
        return false;
    }
    return true;
}

std::vector<std::map<std::string,std::string>> SqliteDatabase::query(const std::string& sql) {
    std::vector<std::map<std::string,std::string>> rows;
    char* err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), cb_rows, &rows, &err) != SQLITE_OK) {
        std::cerr << "SQLite query error: " << (err ? err : "") << "\n";
        if (err) sqlite3_free(err);
    }
    return rows;
}
