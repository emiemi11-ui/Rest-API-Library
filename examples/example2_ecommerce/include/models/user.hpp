#pragma once
#include <string>
#include <sstream>

class User {
public:
    int id;
    std::string name;
    std::string email;
    std::string password_hash;

    User() : id(0) {}
    User(int id, const std::string& name, const std::string& email, const std::string& password_hash = "")
        : id(id), name(name), email(email), password_hash(password_hash) {}

    // Conversie la JSON simplu (fara librarie externa)
    std::string toJson() const {
        std::ostringstream oss;
        oss << "{";
        oss << "\"id\":" << id << ",";
        oss << "\"name\":\"" << escape_json(name) << "\",";
        oss << "\"email\":\"" << escape_json(email) << "\"";
        oss << "}";
        return oss.str();
    }

    // Parsare simpla din JSON
    static User fromJson(const std::string& json) {
        User user;
        // Parsare simplă - în producție ai folosi o librărie
        user.id = extract_int(json, "id");
        user.name = extract_string(json, "name");
        user.email = extract_string(json, "email");
        return user;
    }

private:
    static std::string escape_json(const std::string& str) {
        std::string result;
        for (char c : str) {
            if (c == '"' || c == '\\') result += '\\';
            result += c;
        }
        return result;
    }

    static int extract_int(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return 0;
        pos += search.length();
        return std::stoi(json.substr(pos));
    }

    static std::string extract_string(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":\"";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return "";
        pos += search.length();
        size_t end = json.find("\"", pos);
        if (end == std::string::npos) return "";
        return json.substr(pos, end - pos);
    }
};
