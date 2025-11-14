#pragma once
#include <string>
#include <vector>
#include <ctime>
#include <sstream>
#include <iomanip>

struct OrderItem {
    int id;
    int order_id;
    int product_id;
    int quantity;
    double price_at_purchase;
    std::time_t created_at;

    OrderItem() : id(0), order_id(0), product_id(0), quantity(0),
                  price_at_purchase(0.0), created_at(0) {}

    // Convert to JSON
    std::string toJson() const {
        std::ostringstream oss;
        oss << "{";
        oss << "\"id\":" << id << ",";
        oss << "\"order_id\":" << order_id << ",";
        oss << "\"product_id\":" << product_id << ",";
        oss << "\"quantity\":" << quantity << ",";
        oss << "\"price_at_purchase\":" << std::fixed << std::setprecision(2) << price_at_purchase;
        oss << "}";
        return oss.str();
    }

    // Parse from JSON
    static OrderItem fromJson(const std::string& json) {
        OrderItem item;
        item.product_id = extract_int(json, "product_id");
        item.quantity = extract_int(json, "quantity");
        item.price_at_purchase = extract_double(json, "price_at_purchase");
        return item;
    }

private:
    static int extract_int(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return 0;
        pos += search.length();
        return std::stoi(json.substr(pos));
    }

    static double extract_double(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return 0.0;
        pos += search.length();
        return std::stod(json.substr(pos));
    }
};

struct Order {
    int id;
    int user_id;
    double total_amount;
    std::string status;  // pending, processing, shipped, delivered, cancelled
    std::string shipping_address;
    std::string notes;
    std::time_t created_at;
    std::time_t updated_at;
    std::vector<OrderItem> items;  // Items in this order

    Order() : id(0), user_id(0), total_amount(0.0),
              status("pending"), created_at(0), updated_at(0) {}

    // Convert to JSON
    std::string toJson() const {
        std::ostringstream oss;
        oss << "{";
        oss << "\"id\":" << id << ",";
        oss << "\"user_id\":" << user_id << ",";
        oss << "\"total_amount\":" << std::fixed << std::setprecision(2) << total_amount << ",";
        oss << "\"status\":\"" << escape_json(status) << "\",";
        oss << "\"shipping_address\":\"" << escape_json(shipping_address) << "\",";
        oss << "\"notes\":\"" << escape_json(notes) << "\",";
        oss << "\"items\":[";
        for (size_t i = 0; i < items.size(); ++i) {
            oss << items[i].toJson();
            if (i < items.size() - 1) oss << ",";
        }
        oss << "]";
        oss << "}";
        return oss.str();
    }

    // Parse from JSON
    static Order fromJson(const std::string& json) {
        Order order;
        order.shipping_address = extract_string(json, "shipping_address");
        order.notes = extract_string(json, "notes");

        // Parse items array
        std::string items_search = "\"items\":[";
        size_t items_pos = json.find(items_search);
        if (items_pos != std::string::npos) {
            items_pos += items_search.length();
            size_t items_end = json.find("]", items_pos);
            if (items_end != std::string::npos) {
                std::string items_json = json.substr(items_pos, items_end - items_pos);
                // Parse each item (simplified - assumes items are comma-separated)
                size_t start = 0;
                while (start < items_json.length()) {
                    size_t item_start = items_json.find("{", start);
                    if (item_start == std::string::npos) break;
                    size_t item_end = items_json.find("}", item_start);
                    if (item_end == std::string::npos) break;

                    std::string item_json = items_json.substr(item_start, item_end - item_start + 1);
                    order.items.push_back(OrderItem::fromJson(item_json));

                    start = item_end + 1;
                }
            }
        }

        return order;
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
