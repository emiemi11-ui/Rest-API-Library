// Interactive E-commerce Shop Client (CLI) - Network-capable
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <algorithm>

// ANSI Color Codes
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"

// Simple JSON extractor helper
std::string extract_json_value(const std::string& json, const std::string& key) {
    std::string search_key = "\"" + key + "\"";
    size_t pos = json.find(search_key);
    if (pos == std::string::npos) return "";

    pos = json.find(":", pos);
    if (pos == std::string::npos) return "";
    pos++;

    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;

    if (pos >= json.size()) return "";

    if (json[pos] == '"') {
        pos++;
        size_t end = json.find('"', pos);
        if (end != std::string::npos) {
            return json.substr(pos, end - pos);
        }
    } else {
        size_t end = pos;
        while (end < json.size() && json[end] != ',' && json[end] != '}' &&
               json[end] != ']' && json[end] != '\n') {
            end++;
        }
        std::string value = json.substr(pos, end - pos);
        value.erase(value.find_last_not_of(" \t\n\r") + 1);
        return value;
    }
    return "";
}

// Extract array from JSON
std::vector<std::string> extract_json_array(const std::string& json, const std::string& key) {
    std::vector<std::string> result;
    std::string search_key = "\"" + key + "\"";
    size_t pos = json.find(search_key);
    if (pos == std::string::npos) return result;

    pos = json.find("[", pos);
    if (pos == std::string::npos) return result;

    size_t end = json.find("]", pos);
    if (end == std::string::npos) return result;

    std::string array_content = json.substr(pos + 1, end - pos - 1);

    // Simple object extraction (assumes objects are properly formatted)
    size_t obj_start = 0;
    while ((obj_start = array_content.find("{", obj_start)) != std::string::npos) {
        size_t obj_end = array_content.find("}", obj_start);
        if (obj_end != std::string::npos) {
            result.push_back(array_content.substr(obj_start, obj_end - obj_start + 1));
            obj_start = obj_end + 1;
        } else {
            break;
        }
    }

    return result;
}

// Cart item structure
struct CartItem {
    int product_id;
    std::string name;
    double price;
    int quantity;
};

class ShopClient {
    int sock_;
    std::string server_ip_;
    int server_port_;
    int user_id_;
    std::string username_;
    std::vector<CartItem> cart_;

public:
    ShopClient(const std::string& ip, int port)
        : sock_(-1), server_ip_(ip), server_port_(port), user_id_(-1) {}

    ~ShopClient() {
        disconnect();
    }

    bool connect_to_server() {
        sock_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_ < 0) {
            std::cerr << RED << "[ERROR] Failed to create socket" << RESET << "\n";
            return false;
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port_);

        if (inet_pton(AF_INET, server_ip_.c_str(), &server_addr.sin_addr) <= 0) {
            std::cerr << RED << "[ERROR] Invalid IP address" << RESET << "\n";
            close(sock_);
            sock_ = -1;
            return false;
        }

        if (connect(sock_, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << RED << "[ERROR] Failed to connect to " << server_ip_
                      << ":" << server_port_ << RESET << "\n";
            std::cerr << "       Make sure the server is running!\n";
            close(sock_);
            sock_ = -1;
            return false;
        }

        std::cout << GREEN << "[SUCCESS] Connected to " << server_ip_
                  << ":" << server_port_ << RESET << "\n";
        return true;
    }

    void disconnect() {
        if (sock_ >= 0) {
            close(sock_);
            sock_ = -1;
        }
    }

    bool reconnect() {
        disconnect();
        return connect_to_server();
    }

    std::string send_request(const std::string& method, const std::string& path,
                            const std::string& body = "") {
        if (sock_ < 0) {
            if (!reconnect()) {
                return "";
            }
        }

        std::string request = method + " " + path + " HTTP/1.1\r\n";
        request += "Host: " + server_ip_ + "\r\n";
        request += "Connection: keep-alive\r\n";
        request += "Content-Type: application/json\r\n";
        request += "Content-Length: " + std::to_string(body.size()) + "\r\n";
        request += "\r\n";
        request += body;

        ssize_t sent = send(sock_, request.c_str(), request.size(), 0);
        if (sent < 0) {
            std::cerr << RED << "[ERROR] Failed to send request" << RESET << "\n";
            return "";
        }

        char buffer[16384];
        ssize_t received = recv(sock_, buffer, sizeof(buffer) - 1, 0);
        if (received < 0) {
            std::cerr << RED << "[ERROR] Failed to receive response" << RESET << "\n";
            return "";
        }

        buffer[received] = '\0';
        return std::string(buffer);
    }

    std::string get_response_body(const std::string& response) {
        size_t pos = response.find("\r\n\r\n");
        if (pos != std::string::npos) {
            return response.substr(pos + 4);
        }
        return response;
    }

    void print_header() {
        std::cout << CYAN << BOLD;
        std::cout << "\n╔════════════════════════════════════════════════════════╗\n";
        std::cout << "║        " << YELLOW << "E-COMMERCE SHOP CLIENT" << CYAN << "                    ║\n";
        std::cout << "║        Connected to: " << server_ip_ << ":" << server_port_;
        for (int i = 0; i < (30 - server_ip_.length() - std::to_string(server_port_).length()); i++)
            std::cout << " ";
        std::cout << "║\n";
        if (user_id_ > 0) {
            std::cout << "║        " << GREEN << "Logged in as: " << username_;
            for (int i = 0; i < (34 - username_.length()); i++) std::cout << " ";
            std::cout << CYAN << "║\n";
        }
        std::cout << "╚════════════════════════════════════════════════════════╝\n";
        std::cout << RESET;
    }

    void print_menu() {
        std::cout << "\n" << BOLD << CYAN << "Main Menu:" << RESET << "\n";
        std::cout << "  " << YELLOW << "1." << RESET << " Browse Products\n";
        std::cout << "  " << YELLOW << "2." << RESET << " Search Products\n";
        std::cout << "  " << YELLOW << "3." << RESET << " Filter by Category\n";
        std::cout << "  " << YELLOW << "4." << RESET << " View Cart (" << MAGENTA << cart_.size() << " items" << RESET << ")\n";
        std::cout << "  " << YELLOW << "5." << RESET << " Checkout\n";
        std::cout << "  " << YELLOW << "6." << RESET << " My Orders\n";
        std::cout << "  " << YELLOW << "7." << RESET << " Login/Register\n";
        std::cout << "  " << YELLOW << "8." << RESET << " Exit\n";
        std::cout << "\n" << CYAN << "Choose option: " << RESET;
    }

    void browse_products() {
        std::cout << "\n" << BOLD << "=== Browse Products ===" << RESET << "\n";
        std::cout << "Page (default 1): ";
        std::string page_input;
        std::getline(std::cin, page_input);
        int page = page_input.empty() ? 1 : std::stoi(page_input);

        std::cout << "Limit per page (default 10): ";
        std::string limit_input;
        std::getline(std::cin, limit_input);
        int limit = limit_input.empty() ? 10 : std::stoi(limit_input);

        std::string path = "/api/products?page=" + std::to_string(page) +
                          "&limit=" + std::to_string(limit);
        std::string response = send_request("GET", path);
        std::string body = get_response_body(response);

        if (response.find("200 OK") != std::string::npos) {
            display_products(body);

            // Option to view product details or add to cart
            std::cout << "\n" << CYAN << "Enter product ID to view details (0 to go back): " << RESET;
            std::string id_input;
            std::getline(std::cin, id_input);
            int product_id = std::stoi(id_input);

            if (product_id > 0) {
                view_product_details(product_id);
            }
        } else {
            std::cout << RED << "[ERROR] Failed to fetch products" << RESET << "\n";
        }
    }

    void search_products() {
        std::cout << "\n" << BOLD << "=== Search Products ===" << RESET << "\n";
        std::cout << "Enter search keyword: ";
        std::string keyword;
        std::getline(std::cin, keyword);

        if (keyword.empty()) {
            std::cout << YELLOW << "[WARNING] Keyword cannot be empty" << RESET << "\n";
            return;
        }

        std::string path = "/api/products/search?q=" + keyword;
        std::string response = send_request("GET", path);
        std::string body = get_response_body(response);

        if (response.find("200 OK") != std::string::npos) {
            display_products(body);

            std::cout << "\n" << CYAN << "Enter product ID to view details (0 to go back): " << RESET;
            std::string id_input;
            std::getline(std::cin, id_input);
            int product_id = std::stoi(id_input);

            if (product_id > 0) {
                view_product_details(product_id);
            }
        } else {
            std::cout << RED << "[ERROR] Search failed" << RESET << "\n";
        }
    }

    void filter_by_category() {
        std::cout << "\n" << BOLD << "=== Filter by Category ===" << RESET << "\n";
        std::cout << "Enter category name: ";
        std::string category;
        std::getline(std::cin, category);

        if (category.empty()) {
            std::cout << YELLOW << "[WARNING] Category cannot be empty" << RESET << "\n";
            return;
        }

        std::string path = "/api/products?category=" + category;
        std::string response = send_request("GET", path);
        std::string body = get_response_body(response);

        if (response.find("200 OK") != std::string::npos) {
            display_products(body);

            std::cout << "\n" << CYAN << "Enter product ID to view details (0 to go back): " << RESET;
            std::string id_input;
            std::getline(std::cin, id_input);
            int product_id = std::stoi(id_input);

            if (product_id > 0) {
                view_product_details(product_id);
            }
        } else {
            std::cout << RED << "[ERROR] Filter failed" << RESET << "\n";
        }
    }

    void display_products(const std::string& json) {
        auto products = extract_json_array(json, "products");

        if (products.empty()) {
            std::cout << YELLOW << "[INFO] No products found" << RESET << "\n";
            return;
        }

        std::cout << "\n" << BOLD << "┌──────┬────────────────────────────┬──────────────┬───────────┬───────┐" << RESET << "\n";
        std::cout << BOLD << "│ " << std::setw(4) << "ID" << " │ "
                  << std::setw(26) << "Name" << " │ "
                  << std::setw(12) << "Category" << " │ "
                  << std::setw(9) << "Price" << " │ "
                  << std::setw(5) << "Stock" << " │" << RESET << "\n";
        std::cout << BOLD << "├──────┼────────────────────────────┼──────────────┼───────────┼───────┤" << RESET << "\n";

        for (const auto& product : products) {
            std::string id = extract_json_value(product, "id");
            std::string name = extract_json_value(product, "name");
            std::string category = extract_json_value(product, "category");
            std::string price = extract_json_value(product, "price");
            std::string stock = extract_json_value(product, "stock");

            if (name.length() > 26) name = name.substr(0, 23) + "...";
            if (category.length() > 12) category = category.substr(0, 9) + "...";

            std::cout << "│ " << std::setw(4) << id << " │ "
                      << std::setw(26) << std::left << name << std::right << " │ "
                      << std::setw(12) << category << " │ "
                      << GREEN << "$" << std::setw(8) << price << RESET << " │ "
                      << std::setw(5) << stock << " │\n";
        }
        std::cout << BOLD << "└──────┴────────────────────────────┴──────────────┴───────────┴───────┘" << RESET << "\n";

        std::string total = extract_json_value(json, "total");
        std::string page = extract_json_value(json, "page");
        std::string limit = extract_json_value(json, "limit");

        if (!total.empty()) {
            std::cout << CYAN << "Total products: " << total << " | Page: " << page
                      << " | Per page: " << limit << RESET << "\n";
        }
    }

    void view_product_details(int product_id) {
        std::string path = "/api/products/" + std::to_string(product_id);
        std::string response = send_request("GET", path);
        std::string body = get_response_body(response);

        if (response.find("200 OK") != std::string::npos) {
            std::cout << "\n" << BOLD << CYAN << "╔═══════════════════════════════════════════════╗" << RESET << "\n";
            std::cout << BOLD << CYAN << "║         PRODUCT DETAILS                       ║" << RESET << "\n";
            std::cout << BOLD << CYAN << "╚═══════════════════════════════════════════════╝" << RESET << "\n\n";

            std::string name = extract_json_value(body, "name");
            std::string category = extract_json_value(body, "category");
            std::string price = extract_json_value(body, "price");
            std::string stock = extract_json_value(body, "stock");
            std::string description = extract_json_value(body, "description");

            std::cout << BOLD << "Name:        " << RESET << name << "\n";
            std::cout << BOLD << "Category:    " << RESET << category << "\n";
            std::cout << BOLD << "Price:       " << RESET << GREEN << "$" << price << RESET << "\n";
            std::cout << BOLD << "Stock:       " << RESET << stock << " units\n";
            std::cout << BOLD << "Description: " << RESET << description << "\n";

            // Add to cart option
            std::cout << "\n" << CYAN << "Add to cart? (y/n): " << RESET;
            std::string choice;
            std::getline(std::cin, choice);

            if (choice == "y" || choice == "Y") {
                std::cout << "Quantity: ";
                std::string qty_input;
                std::getline(std::cin, qty_input);
                int quantity = std::stoi(qty_input);

                if (quantity > 0 && quantity <= std::stoi(stock)) {
                    add_to_cart(product_id, name, std::stod(price), quantity);
                } else {
                    std::cout << RED << "[ERROR] Invalid quantity" << RESET << "\n";
                }
            }
        } else {
            std::cout << RED << "[ERROR] Product not found" << RESET << "\n";
        }
    }

    void add_to_cart(int product_id, const std::string& name, double price, int quantity) {
        // Check if product already in cart
        for (auto& item : cart_) {
            if (item.product_id == product_id) {
                item.quantity += quantity;
                std::cout << GREEN << "[SUCCESS] Updated quantity in cart" << RESET << "\n";
                return;
            }
        }

        CartItem item{product_id, name, price, quantity};
        cart_.push_back(item);
        std::cout << GREEN << "[SUCCESS] Added to cart!" << RESET << "\n";
    }

    void view_cart() {
        std::cout << "\n" << BOLD << "=== Shopping Cart ===" << RESET << "\n";

        if (cart_.empty()) {
            std::cout << YELLOW << "[INFO] Cart is empty" << RESET << "\n";
            return;
        }

        std::cout << "\n" << BOLD << "┌──────┬────────────────────────────┬───────────┬──────────┬────────────┐" << RESET << "\n";
        std::cout << BOLD << "│ " << std::setw(4) << "ID" << " │ "
                  << std::setw(26) << "Name" << " │ "
                  << std::setw(9) << "Price" << " │ "
                  << std::setw(8) << "Quantity" << " │ "
                  << std::setw(10) << "Subtotal" << " │" << RESET << "\n";
        std::cout << BOLD << "├──────┼────────────────────────────┼───────────┼──────────┼────────────┤" << RESET << "\n";

        double total = 0.0;
        for (const auto& item : cart_) {
            double subtotal = item.price * item.quantity;
            total += subtotal;

            std::string name = item.name;
            if (name.length() > 26) name = name.substr(0, 23) + "...";

            std::cout << "│ " << std::setw(4) << item.product_id << " │ "
                      << std::setw(26) << std::left << name << std::right << " │ "
                      << GREEN << "$" << std::setw(8) << std::fixed << std::setprecision(2)
                      << item.price << RESET << " │ "
                      << std::setw(8) << item.quantity << " │ "
                      << GREEN << "$" << std::setw(9) << subtotal << RESET << " │\n";
        }
        std::cout << BOLD << "└──────┴────────────────────────────┴───────────┴──────────┴────────────┘" << RESET << "\n";
        std::cout << BOLD << GREEN << "Total: $" << std::fixed << std::setprecision(2) << total << RESET << "\n";

        // Cart management options
        std::cout << "\n" << CYAN << "Options: (r)emove item, (c)lear cart, (b)ack: " << RESET;
        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "r" || choice == "R") {
            std::cout << "Enter product ID to remove: ";
            std::string id_input;
            std::getline(std::cin, id_input);
            int product_id = std::stoi(id_input);

            auto it = std::remove_if(cart_.begin(), cart_.end(),
                [product_id](const CartItem& item) { return item.product_id == product_id; });

            if (it != cart_.end()) {
                cart_.erase(it, cart_.end());
                std::cout << GREEN << "[SUCCESS] Item removed from cart" << RESET << "\n";
            } else {
                std::cout << RED << "[ERROR] Item not found in cart" << RESET << "\n";
            }
        } else if (choice == "c" || choice == "C") {
            cart_.clear();
            std::cout << GREEN << "[SUCCESS] Cart cleared" << RESET << "\n";
        }
    }

    void checkout() {
        if (cart_.empty()) {
            std::cout << YELLOW << "[WARNING] Cart is empty. Add products first!" << RESET << "\n";
            return;
        }

        if (user_id_ < 0) {
            std::cout << YELLOW << "[WARNING] Please login first to checkout" << RESET << "\n";
            return;
        }

        std::cout << "\n" << BOLD << "=== Checkout ===" << RESET << "\n";

        double total = 0.0;
        for (const auto& item : cart_) {
            total += item.price * item.quantity;
        }

        std::cout << BOLD << "Total Amount: " << GREEN << "$" << std::fixed
                  << std::setprecision(2) << total << RESET << "\n\n";

        std::cout << "Shipping Address: ";
        std::string address;
        std::getline(std::cin, address);

        // Build order JSON
        std::stringstream json;
        json << "{\"user_id\":" << user_id_
             << ",\"total_amount\":" << std::fixed << std::setprecision(2) << total
             << ",\"status\":\"pending\""
             << ",\"address\":\"" << address << "\""
             << ",\"items\":[";

        for (size_t i = 0; i < cart_.size(); i++) {
            if (i > 0) json << ",";
            json << "{\"product_id\":" << cart_[i].product_id
                 << ",\"quantity\":" << cart_[i].quantity
                 << ",\"price\":" << std::fixed << std::setprecision(2) << cart_[i].price << "}";
        }
        json << "]}";

        std::string response = send_request("POST", "/api/orders", json.str());
        std::string body = get_response_body(response);

        if (response.find("201 Created") != std::string::npos ||
            response.find("200 OK") != std::string::npos) {
            std::string order_id = extract_json_value(body, "id");
            std::cout << "\n" << GREEN << "╔═══════════════════════════════════════════╗" << RESET << "\n";
            std::cout << GREEN << "║  ORDER PLACED SUCCESSFULLY!               ║" << RESET << "\n";
            std::cout << GREEN << "╚═══════════════════════════════════════════╝" << RESET << "\n";
            std::cout << "Order ID: " << BOLD << order_id << RESET << "\n";
            std::cout << "Total: " << GREEN << "$" << total << RESET << "\n";

            cart_.clear();
        } else {
            std::cout << RED << "[ERROR] Failed to create order" << RESET << "\n";
            std::cout << "Response: " << body << "\n";
        }
    }

    void view_orders() {
        if (user_id_ < 0) {
            std::cout << YELLOW << "[WARNING] Please login first" << RESET << "\n";
            return;
        }

        std::cout << "\n" << BOLD << "=== My Orders ===" << RESET << "\n";

        std::string path = "/api/orders?user_id=" + std::to_string(user_id_);
        std::string response = send_request("GET", path);
        std::string body = get_response_body(response);

        if (response.find("200 OK") != std::string::npos) {
            auto orders = extract_json_array(body, "orders");

            if (orders.empty()) {
                std::cout << YELLOW << "[INFO] No orders found" << RESET << "\n";
                return;
            }

            std::cout << "\n" << BOLD << "┌──────┬──────────────┬─────────────┬────────────┐" << RESET << "\n";
            std::cout << BOLD << "│ " << std::setw(4) << "ID" << " │ "
                      << std::setw(12) << "Total" << " │ "
                      << std::setw(11) << "Status" << " │ "
                      << std::setw(10) << "Date" << " │" << RESET << "\n";
            std::cout << BOLD << "├──────┼──────────────┼─────────────┼────────────┤" << RESET << "\n";

            for (const auto& order : orders) {
                std::string id = extract_json_value(order, "id");
                std::string total = extract_json_value(order, "total_amount");
                std::string status = extract_json_value(order, "status");
                std::string date = extract_json_value(order, "created_at");

                if (date.length() > 10) date = date.substr(0, 10);

                std::string status_color = YELLOW;
                if (status == "completed") status_color = GREEN;
                else if (status == "cancelled") status_color = RED;

                std::cout << "│ " << std::setw(4) << id << " │ "
                          << GREEN << "$" << std::setw(11) << total << RESET << " │ "
                          << status_color << std::setw(11) << status << RESET << " │ "
                          << std::setw(10) << date << " │\n";
            }
            std::cout << BOLD << "└──────┴──────────────┴─────────────┴────────────┘" << RESET << "\n";

            std::cout << "\n" << CYAN << "Enter order ID to view details (0 to go back): " << RESET;
            std::string id_input;
            std::getline(std::cin, id_input);
            int order_id = std::stoi(id_input);

            if (order_id > 0) {
                view_order_details(order_id);
            }
        } else {
            std::cout << RED << "[ERROR] Failed to fetch orders" << RESET << "\n";
        }
    }

    void view_order_details(int order_id) {
        std::string path = "/api/orders/" + std::to_string(order_id);
        std::string response = send_request("GET", path);
        std::string body = get_response_body(response);

        if (response.find("200 OK") != std::string::npos) {
            std::cout << "\n" << BOLD << CYAN << "╔═══════════════════════════════════════════════╗" << RESET << "\n";
            std::cout << BOLD << CYAN << "║         ORDER DETAILS                         ║" << RESET << "\n";
            std::cout << BOLD << CYAN << "╚═══════════════════════════════════════════════╝" << RESET << "\n\n";

            std::string id = extract_json_value(body, "id");
            std::string total = extract_json_value(body, "total_amount");
            std::string status = extract_json_value(body, "status");
            std::string address = extract_json_value(body, "address");
            std::string date = extract_json_value(body, "created_at");

            std::cout << BOLD << "Order ID:    " << RESET << id << "\n";
            std::cout << BOLD << "Date:        " << RESET << date << "\n";
            std::cout << BOLD << "Status:      " << RESET << status << "\n";
            std::cout << BOLD << "Total:       " << RESET << GREEN << "$" << total << RESET << "\n";
            std::cout << BOLD << "Address:     " << RESET << address << "\n\n";

            // Try to get order items
            std::string items_path = "/api/orders/" + std::to_string(order_id) + "/items";
            std::string items_response = send_request("GET", items_path);
            std::string items_body = get_response_body(items_response);

            if (items_response.find("200 OK") != std::string::npos) {
                auto items = extract_json_array(items_body, "items");
                if (!items.empty()) {
                    std::cout << BOLD << "Items:" << RESET << "\n";
                    for (const auto& item : items) {
                        std::string product_name = extract_json_value(item, "product_name");
                        std::string quantity = extract_json_value(item, "quantity");
                        std::string price = extract_json_value(item, "price");

                        std::cout << "  - " << product_name << " x" << quantity
                                  << " @ $" << price << "\n";
                    }
                }
            }
        } else {
            std::cout << RED << "[ERROR] Order not found" << RESET << "\n";
        }
    }

    void login_register() {
        std::cout << "\n" << BOLD << "=== Authentication ===" << RESET << "\n";
        std::cout << "1. Login\n";
        std::cout << "2. Register\n";
        std::cout << "Choose option: ";

        std::string choice;
        std::getline(std::cin, choice);

        if (choice == "1") {
            login();
        } else if (choice == "2") {
            register_user();
        } else {
            std::cout << YELLOW << "[WARNING] Invalid choice" << RESET << "\n";
        }
    }

    void login() {
        std::cout << "\n" << BOLD << "=== Login ===" << RESET << "\n";
        std::cout << "Username: ";
        std::string username;
        std::getline(std::cin, username);

        std::cout << "Password: ";
        std::string password;
        std::getline(std::cin, password);

        std::string json = "{\"username\":\"" + username + "\",\"password\":\"" + password + "\"}";
        std::string response = send_request("POST", "/api/auth/login", json);
        std::string body = get_response_body(response);

        if (response.find("200 OK") != std::string::npos) {
            std::string user_id_str = extract_json_value(body, "user_id");
            if (!user_id_str.empty()) {
                user_id_ = std::stoi(user_id_str);
                username_ = username;
                std::cout << GREEN << "[SUCCESS] Logged in successfully!" << RESET << "\n";
            } else {
                std::cout << RED << "[ERROR] Invalid response from server" << RESET << "\n";
            }
        } else {
            std::cout << RED << "[ERROR] Login failed. Check credentials." << RESET << "\n";
        }
    }

    void register_user() {
        std::cout << "\n" << BOLD << "=== Register ===" << RESET << "\n";
        std::cout << "Username: ";
        std::string username;
        std::getline(std::cin, username);

        std::cout << "Email: ";
        std::string email;
        std::getline(std::cin, email);

        std::cout << "Password: ";
        std::string password;
        std::getline(std::cin, password);

        std::string json = "{\"username\":\"" + username + "\",\"email\":\"" + email +
                          "\",\"password\":\"" + password + "\"}";
        std::string response = send_request("POST", "/api/users", json);
        std::string body = get_response_body(response);

        if (response.find("201 Created") != std::string::npos ||
            response.find("200 OK") != std::string::npos) {
            std::string user_id_str = extract_json_value(body, "id");
            if (!user_id_str.empty()) {
                user_id_ = std::stoi(user_id_str);
                username_ = username;
                std::cout << GREEN << "[SUCCESS] Registration successful! You are now logged in." << RESET << "\n";
            } else {
                std::cout << GREEN << "[SUCCESS] Registration successful! Please login." << RESET << "\n";
            }
        } else {
            std::cout << RED << "[ERROR] Registration failed." << RESET << "\n";
            std::cout << "Response: " << body << "\n";
        }
    }

    void run() {
        print_header();

        while (true) {
            print_menu();

            std::string choice;
            std::getline(std::cin, choice);

            if (choice == "1") {
                browse_products();
            } else if (choice == "2") {
                search_products();
            } else if (choice == "3") {
                filter_by_category();
            } else if (choice == "4") {
                view_cart();
            } else if (choice == "5") {
                checkout();
            } else if (choice == "6") {
                view_orders();
            } else if (choice == "7") {
                login_register();
            } else if (choice == "8") {
                std::cout << "\n" << CYAN << "Thank you for shopping! Goodbye!" << RESET << "\n";
                break;
            } else {
                std::cout << YELLOW << "[WARNING] Invalid option. Try again." << RESET << "\n";
            }
        }
    }
};

int main(int argc, char** argv) {
    std::string server_ip = "127.0.0.1";
    int server_port = 8080;

    if (argc < 3) {
        std::cout << YELLOW << "[INFO] Usage: " << argv[0] << " <server_ip> <server_port>" << RESET << "\n";
        std::cout << YELLOW << "[INFO] Using defaults: " << server_ip << ":" << server_port << RESET << "\n\n";
    } else {
        server_ip = argv[1];
        server_port = std::atoi(argv[2]);
    }

    ShopClient client(server_ip, server_port);

    if (!client.connect_to_server()) {
        std::cerr << RED << "[FATAL] Could not connect to server. Exiting..." << RESET << "\n";
        return 1;
    }

    client.run();

    return 0;
}
