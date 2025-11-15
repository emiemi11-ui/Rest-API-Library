#pragma once
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>

struct Product {
    int id;
    std::string sku;
    std::string name;
    std::string description;
    double price;
    int stock_quantity;
    std::string category;
    std::string image_url;
    bool is_active;
    std::time_t created_at;
    std::time_t updated_at;
    int created_by;  // user id

    Product() : id(0), price(0.0), stock_quantity(0), is_active(true),
                created_at(0), updated_at(0), created_by(0) {}

    // JSON conversion
    std::string toJson() const {
        std::ostringstream oss;
        oss << "{";
        oss << "\"id\":" << id << ",";
        oss << "\"sku\":\"" << escape_json(sku) << "\",";
        oss << "\"name\":\"" << escape_json(name) << "\",";
        oss << "\"description\":\"" << escape_json(description) << "\",";
        oss << std::fixed << std::setprecision(2);
        oss << "\"price\":" << price << ",";
        oss << "\"stock_quantity\":" << stock_quantity << ",";
        oss << "\"category\":\"" << escape_json(category) << "\",";
        oss << "\"image_url\":\"" << escape_json(image_url) << "\",";
        oss << "\"is_active\":" << (is_active ? "true" : "false") << ",";
        oss << "\"created_at\":" << created_at << ",";
        oss << "\"updated_at\":" << updated_at << ",";
        oss << "\"created_by\":" << created_by;
        oss << "}";
        return oss.str();
    }

    static Product fromJson(const std::string& json) {
        Product product;
        product.id = extract_int(json, "id");
        product.sku = extract_string(json, "sku");
        product.name = extract_string(json, "name");
        product.description = extract_string(json, "description");
        product.price = extract_double(json, "price");
        product.stock_quantity = extract_int(json, "stock_quantity");
        product.category = extract_string(json, "category");
        product.image_url = extract_string(json, "image_url");
        product.is_active = extract_bool(json, "is_active");
        product.created_by = extract_int(json, "created_by");
        return product;
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
        // Skip whitespace
        while (pos < json.length() && std::isspace(json[pos])) pos++;
        return std::stoi(json.substr(pos));
    }

    static double extract_double(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return 0.0;
        pos += search.length();
        // Skip whitespace
        while (pos < json.length() && std::isspace(json[pos])) pos++;
        return std::stod(json.substr(pos));
    }

    static bool extract_bool(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return true; // default is_active = true
        pos += search.length();
        // Skip whitespace
        while (pos < json.length() && std::isspace(json[pos])) pos++;
        return json.substr(pos, 4) == "true";
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
