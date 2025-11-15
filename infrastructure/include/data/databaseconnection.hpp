#pragma once
#include "data/idatabase.hpp"
#include <memory>
#include <map>
#include <string>

/// Container simplu pentru o implementare IDatabase + config.
/// Se ocupă doar să mențină instanța și configurația.
class DatabaseConnection {
    std::unique_ptr<IDatabase> impl;
    std::map<std::string,std::string> cfg;
public:
    DatabaseConnection(std::unique_ptr<IDatabase> dbImpl, std::map<std::string,std::string> config)
        : impl(std::move(dbImpl)), cfg(std::move(config)) {}

    bool connect() { return impl->connect(cfg); }
    void disconnect() { impl->disconnect(); }
    IDatabase& db() { return *impl; }
};
