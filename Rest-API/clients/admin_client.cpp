// Admin Dashboard Client - Network-capable TUI for REST API monitoring
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <algorithm>
#include <ctime>

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
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"

// Screen control
#define CLEAR_SCREEN "\033[2J\033[H"
#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"

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

// Extract JSON body from HTTP response
std::string extract_body(const std::string& response) {
    size_t pos = response.find("\r\n\r\n");
    if (pos != std::string::npos) {
        return response.substr(pos + 4);
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

    // Simple object extraction
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

// Print table border
void print_line(int width, char c = '─') {
    for (int i = 0; i < width; i++) {
        std::cout << c;
    }
    std::cout << "\n";
}

// Print centered text
void print_centered(const std::string& text, int width, const std::string& color = "") {
    int padding = (width - text.length()) / 2;
    std::cout << color;
    for (int i = 0; i < padding; i++) std::cout << " ";
    std::cout << text;
    for (int i = 0; i < (width - text.length() - padding); i++) std::cout << " ";
    std::cout << RESET << "\n";
}

// Get current timestamp
std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

class AdminDashboard {
    int sock_;
    std::string server_ip_;
    int server_port_;
    int refresh_interval_;
    std::chrono::steady_clock::time_point server_start_time_;
    bool connected_;

public:
    AdminDashboard(const std::string& ip, int port, int refresh = 0)
        : sock_(-1), server_ip_(ip), server_port_(port),
          refresh_interval_(refresh), connected_(false) {
        server_start_time_ = std::chrono::steady_clock::now();
    }

    ~AdminDashboard() {
        disconnect();
    }

    bool connect_to_server() {
        sock_ = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_ < 0) {
            return false;
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port_);

        if (inet_pton(AF_INET, server_ip_.c_str(), &server_addr.sin_addr) <= 0) {
            close(sock_);
            sock_ = -1;
            return false;
        }

        if (connect(sock_, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(sock_);
            sock_ = -1;
            return false;
        }

        connected_ = true;
        return true;
    }

    void disconnect() {
        if (sock_ >= 0) {
            close(sock_);
            sock_ = -1;
        }
        connected_ = false;
    }

    std::string send_request(const std::string& method, const std::string& path,
                            const std::string& body = "") {
        // Reconnect for each request to avoid connection issues
        if (sock_ < 0) {
            if (!connect_to_server()) {
                return "";
            }
        }

        std::string request = method + " " + path + " HTTP/1.1\r\n";
        request += "Host: " + server_ip_ + "\r\n";
        request += "Content-Length: " + std::to_string(body.size()) + "\r\n";
        request += "Connection: close\r\n";
        request += "\r\n";
        request += body;

        ssize_t sent = send(sock_, request.c_str(), request.size(), 0);
        if (sent < 0) {
            disconnect();
            return "";
        }

        // Receive response
        std::string response;
        char buffer[4096];
        ssize_t received;

        while ((received = recv(sock_, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[received] = '\0';
            response += buffer;
        }

        disconnect();
        return response;
    }

    void display_header() {
        std::cout << CLEAR_SCREEN;
        std::cout << BOLD << CYAN;
        std::cout << "╔════════════════════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                                                                                ║\n";
        std::cout << "║             " << GREEN << "★★★ REST API ADMIN DASHBOARD ★★★" << CYAN << "                            ║\n";
        std::cout << "║                                                                                ║\n";
        std::cout << "╚════════════════════════════════════════════════════════════════════════════════╝\n";
        std::cout << RESET;

        std::cout << YELLOW << "  Server: " << RESET << server_ip_ << ":" << server_port_
                  << "  │  " << YELLOW << "Time: " << RESET << get_timestamp();

        if (refresh_interval_ > 0) {
            std::cout << "  │  " << YELLOW << "Auto-refresh: " << RESET << refresh_interval_ << "s";
        }
        std::cout << "\n";
        print_line(84, '─');
    }

    void display_server_status() {
        std::cout << BOLD << BLUE << "\n┌─ SERVER STATUS " << RESET;
        print_line(66, '─');

        // Get health check
        std::string response = send_request("GET", "/health?user_id=1");
        std::string body = extract_body(response);
        std::string status = extract_json_value(body, "status");

        auto now = std::chrono::steady_clock::now();
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - server_start_time_);
        int hours = uptime.count() / 3600;
        int minutes = (uptime.count() % 3600) / 60;
        int seconds = uptime.count() % 60;

        std::cout << "  Health Status: ";
        if (status == "OK") {
            std::cout << GREEN << "● HEALTHY" << RESET;
        } else {
            std::cout << RED << "● DOWN" << RESET;
        }
        std::cout << "  │  Uptime: " << CYAN << hours << "h " << minutes << "m " << seconds << "s" << RESET;
        std::cout << "\n";
    }

    void display_statistics() {
        std::cout << BOLD << BLUE << "\n┌─ SYSTEM STATISTICS " << RESET;
        print_line(62, '─');

        // Get statistics
        std::string stats_response = send_request("GET", "/api/orders/stats?user_id=1");
        std::string stats_body = extract_body(stats_response);

        std::string total_orders = extract_json_value(stats_body, "total_orders");
        std::string total_revenue = extract_json_value(stats_body, "total_revenue");
        std::string pending = extract_json_value(stats_body, "pending_orders");
        std::string completed = extract_json_value(stats_body, "completed_orders");
        std::string cancelled = extract_json_value(stats_body, "cancelled_orders");

        // Get counts
        std::string users_response = send_request("GET", "/api/users?user_id=1");
        std::string users_body = extract_body(users_response);
        auto users = extract_json_array(users_body, "users");

        std::string products_response = send_request("GET", "/api/products?user_id=1");
        std::string products_body = extract_body(products_response);
        auto products = extract_json_array(products_body, "products");

        std::cout << "  ┌────────────────────────┬────────────────────────┬────────────────────────┐\n";
        std::cout << "  │  " << CYAN << "Total Users       " << RESET << "│  "
                  << CYAN << "Total Products    " << RESET << "│  "
                  << CYAN << "Total Orders      " << RESET << "│\n";
        std::cout << "  │  " << GREEN << std::setw(17) << std::left << users.size() << RESET << " │  "
                  << GREEN << std::setw(17) << std::left << products.size() << RESET << " │  "
                  << GREEN << std::setw(17) << std::left << total_orders << RESET << " │\n";
        std::cout << "  └────────────────────────┴────────────────────────┴────────────────────────┘\n";

        std::cout << "\n  " << BOLD << "Orders by Status:" << RESET << "\n";
        std::cout << "  ┌────────────────┬────────────────┬────────────────┬────────────────┐\n";
        std::cout << "  │ " << YELLOW << "Pending        " << RESET
                  << "│ " << BLUE << "Processing     " << RESET
                  << "│ " << GREEN << "Completed      " << RESET
                  << "│ " << RED << "Cancelled      " << RESET << "│\n";
        std::cout << "  │ " << std::setw(14) << std::left << pending << " │ "
                  << std::setw(14) << std::left << "N/A" << " │ "
                  << std::setw(14) << std::left << completed << " │ "
                  << std::setw(14) << std::left << cancelled << " │\n";
        std::cout << "  └────────────────┴────────────────┴────────────────┴────────────────┘\n";

        std::cout << "\n  " << BOLD << "Revenue:" << RESET << " " << GREEN << "$"
                  << std::fixed << std::setprecision(2) << total_revenue << RESET << "\n";
    }

    void display_recent_orders() {
        std::cout << BOLD << BLUE << "\n┌─ RECENT ORDERS (Last 10) " << RESET;
        print_line(56, '─');

        std::string orders_response = send_request("GET", "/api/orders?user_id=1");
        std::string orders_body = extract_body(orders_response);
        auto orders = extract_json_array(orders_body, "orders");

        if (orders.empty()) {
            std::cout << "  " << YELLOW << "No orders found" << RESET << "\n";
            return;
        }

        std::cout << "  ┌──────┬──────────┬─────────────────────┬──────────────┬──────────────┐\n";
        std::cout << "  │ " << BOLD << "ID" << RESET
                  << "   │ " << BOLD << "User ID" << RESET
                  << "  │ " << BOLD << "Created At" << RESET
                  << "          │ " << BOLD << "Total" << RESET
                  << "        │ " << BOLD << "Status" << RESET << "       │\n";
        std::cout << "  ├──────┼──────────┼─────────────────────┼──────────────┼──────────────┤\n";

        int count = 0;
        for (const auto& order_json : orders) {
            if (count >= 10) break;

            std::string id = extract_json_value(order_json, "id");
            std::string user_id = extract_json_value(order_json, "user_id");
            std::string created = extract_json_value(order_json, "created_at");
            std::string total = extract_json_value(order_json, "total_amount");
            std::string status = extract_json_value(order_json, "status");

            // Truncate timestamp
            if (created.length() > 19) created = created.substr(0, 19);

            // Color code status
            std::string status_color;
            if (status == "pending") status_color = YELLOW;
            else if (status == "completed") status_color = GREEN;
            else if (status == "cancelled") status_color = RED;
            else status_color = CYAN;

            std::cout << "  │ " << std::setw(4) << std::left << id << " │ "
                      << std::setw(8) << std::left << user_id << " │ "
                      << std::setw(19) << std::left << created << " │ "
                      << "$" << std::setw(11) << std::left << total << " │ "
                      << status_color << std::setw(12) << std::left << status << RESET << " │\n";
            count++;
        }

        std::cout << "  └──────┴──────────┴─────────────────────┴──────────────┴──────────────┘\n";
    }

    void display_low_stock_products() {
        std::cout << BOLD << BLUE << "\n┌─ LOW STOCK ALERT (Stock < 10) " << RESET;
        print_line(50, '─');

        std::string products_response = send_request("GET", "/api/products/low-stock?user_id=1");
        std::string products_body = extract_body(products_response);
        auto products = extract_json_array(products_body, "products");

        if (products.empty()) {
            std::cout << "  " << GREEN << "✓ All products have adequate stock" << RESET << "\n";
            return;
        }

        std::cout << "  ┌──────┬──────────────────────────────────────┬────────┬─────────────┐\n";
        std::cout << "  │ " << BOLD << "ID" << RESET
                  << "   │ " << BOLD << "Name" << RESET
                  << "                                 │ " << BOLD << "Stock" << RESET
                  << "  │ " << BOLD << "Price" << RESET << "       │\n";
        std::cout << "  ├──────┼──────────────────────────────────────┼────────┼─────────────┤\n";

        for (const auto& product_json : products) {
            std::string id = extract_json_value(product_json, "id");
            std::string name = extract_json_value(product_json, "name");
            std::string stock = extract_json_value(product_json, "stock");
            std::string price = extract_json_value(product_json, "price");

            // Truncate name if too long
            if (name.length() > 36) name = name.substr(0, 33) + "...";

            std::string stock_color = (std::stoi(stock) < 5) ? RED : YELLOW;

            std::cout << "  │ " << std::setw(4) << std::left << id << " │ "
                      << std::setw(36) << std::left << name << " │ "
                      << stock_color << std::setw(6) << std::left << stock << RESET << " │ "
                      << "$" << std::setw(10) << std::left << price << " │\n";
        }

        std::cout << "  └──────┴──────────────────────────────────────┴────────┴─────────────┘\n";
    }

    void display_menu() {
        std::cout << BOLD << MAGENTA << "\n┌─ QUICK ACTIONS " << RESET;
        print_line(66, '─');
        std::cout << "  " << CYAN << "[1]" << RESET << " View All Users      "
                  << CYAN << "[2]" << RESET << " View All Products\n";
        std::cout << "  " << CYAN << "[3]" << RESET << " Update Order Status "
                  << CYAN << "[4]" << RESET << " View Order Details\n";
        std::cout << "  " << CYAN << "[R]" << RESET << " Refresh Dashboard   "
                  << CYAN << "[Q]" << RESET << " Quit\n";
        print_line(84, '─');
    }

    void display_dashboard() {
        display_header();
        display_server_status();
        display_statistics();
        display_recent_orders();
        display_low_stock_products();
        display_menu();
    }

    void view_all_users() {
        std::cout << CLEAR_SCREEN;
        std::cout << BOLD << CYAN << "\n=== ALL USERS ===" << RESET << "\n\n";

        std::string response = send_request("GET", "/api/users?user_id=1");
        std::string body = extract_body(response);
        auto users = extract_json_array(body, "users");

        if (users.empty()) {
            std::cout << RED << "No users found" << RESET << "\n";
            return;
        }

        std::cout << "┌──────┬──────────────────────┬────────────────────────────────────┬──────────┐\n";
        std::cout << "│ " << BOLD << "ID" << RESET
                  << "   │ " << BOLD << "Username" << RESET
                  << "             │ " << BOLD << "Email" << RESET
                  << "                              │ " << BOLD << "Role" << RESET << "     │\n";
        std::cout << "├──────┼──────────────────────┼────────────────────────────────────┼──────────┤\n";

        for (const auto& user_json : users) {
            std::string id = extract_json_value(user_json, "id");
            std::string username = extract_json_value(user_json, "username");
            std::string email = extract_json_value(user_json, "email");
            std::string role = extract_json_value(user_json, "role");

            if (username.length() > 20) username = username.substr(0, 17) + "...";
            if (email.length() > 34) email = email.substr(0, 31) + "...";

            std::string role_color = (role == "admin") ? GREEN : WHITE;

            std::cout << "│ " << std::setw(4) << std::left << id << " │ "
                      << std::setw(20) << std::left << username << " │ "
                      << std::setw(34) << std::left << email << " │ "
                      << role_color << std::setw(8) << std::left << role << RESET << " │\n";
        }

        std::cout << "└──────┴──────────────────────┴────────────────────────────────────┴──────────┘\n";
        std::cout << "\nTotal users: " << GREEN << users.size() << RESET << "\n";
    }

    void view_all_products() {
        std::cout << CLEAR_SCREEN;
        std::cout << BOLD << CYAN << "\n=== ALL PRODUCTS ===" << RESET << "\n\n";

        std::string response = send_request("GET", "/api/products?user_id=1");
        std::string body = extract_body(response);
        auto products = extract_json_array(body, "products");

        if (products.empty()) {
            std::cout << RED << "No products found" << RESET << "\n";
            return;
        }

        std::cout << "┌──────┬────────────────────────────────┬─────────────┬────────┬──────────┐\n";
        std::cout << "│ " << BOLD << "ID" << RESET
                  << "   │ " << BOLD << "Name" << RESET
                  << "                           │ " << BOLD << "Price" << RESET
                  << "       │ " << BOLD << "Stock" << RESET
                  << "  │ " << BOLD << "Category" << RESET << " │\n";
        std::cout << "├──────┼────────────────────────────────┼─────────────┼────────┼──────────┤\n";

        for (const auto& product_json : products) {
            std::string id = extract_json_value(product_json, "id");
            std::string name = extract_json_value(product_json, "name");
            std::string price = extract_json_value(product_json, "price");
            std::string stock = extract_json_value(product_json, "stock");
            std::string category = extract_json_value(product_json, "category");

            if (name.length() > 30) name = name.substr(0, 27) + "...";
            if (category.length() > 8) category = category.substr(0, 8);

            int stock_val = std::stoi(stock);
            std::string stock_color = (stock_val < 10) ? YELLOW : GREEN;
            if (stock_val < 5) stock_color = RED;

            std::cout << "│ " << std::setw(4) << std::left << id << " │ "
                      << std::setw(30) << std::left << name << " │ "
                      << "$" << std::setw(10) << std::right << price << " │ "
                      << stock_color << std::setw(6) << std::right << stock << RESET << " │ "
                      << std::setw(8) << std::left << category << " │\n";
        }

        std::cout << "└──────┴────────────────────────────────┴─────────────┴────────┴──────────┘\n";
        std::cout << "\nTotal products: " << GREEN << products.size() << RESET << "\n";
    }

    void update_order_status() {
        std::cout << CLEAR_SCREEN;
        std::cout << BOLD << CYAN << "\n=== UPDATE ORDER STATUS ===" << RESET << "\n\n";

        std::cout << "Enter Order ID: ";
        int order_id;
        std::cin >> order_id;

        std::cout << "\nSelect new status:\n";
        std::cout << "  1. pending\n";
        std::cout << "  2. processing\n";
        std::cout << "  3. shipped\n";
        std::cout << "  4. delivered\n";
        std::cout << "  5. cancelled\n";
        std::cout << "\nChoice: ";

        int choice;
        std::cin >> choice;

        std::string status;
        switch (choice) {
            case 1: status = "pending"; break;
            case 2: status = "processing"; break;
            case 3: status = "shipped"; break;
            case 4: status = "delivered"; break;
            case 5: status = "cancelled"; break;
            default:
                std::cout << RED << "\nInvalid choice!" << RESET << "\n";
                return;
        }

        std::string body = "{\"status\":\"" + status + "\"}";
        std::string path = "/api/orders/" + std::to_string(order_id) + "/status?user_id=1";
        std::string response = send_request("PUT", path, body);
        std::string response_body = extract_body(response);

        if (response.find("200 OK") != std::string::npos) {
            std::cout << GREEN << "\n✓ Order status updated successfully!" << RESET << "\n";
        } else {
            std::cout << RED << "\n✗ Failed to update order status" << RESET << "\n";
            std::string error = extract_json_value(response_body, "error");
            if (!error.empty()) {
                std::cout << "Error: " << error << "\n";
            }
        }
    }

    void view_order_details() {
        std::cout << CLEAR_SCREEN;
        std::cout << BOLD << CYAN << "\n=== ORDER DETAILS ===" << RESET << "\n\n";

        std::cout << "Enter Order ID: ";
        int order_id;
        std::cin >> order_id;

        std::string path = "/api/orders/" + std::to_string(order_id) + "?user_id=1";
        std::string response = send_request("GET", path);
        std::string body = extract_body(response);

        if (response.find("404") != std::string::npos) {
            std::cout << RED << "\n✗ Order not found" << RESET << "\n";
            return;
        }

        std::string id = extract_json_value(body, "id");
        std::string user_id = extract_json_value(body, "user_id");
        std::string status = extract_json_value(body, "status");
        std::string total = extract_json_value(body, "total_amount");
        std::string created = extract_json_value(body, "created_at");
        std::string shipping = extract_json_value(body, "shipping_address");

        std::cout << "Order ID: " << CYAN << id << RESET << "\n";
        std::cout << "User ID: " << user_id << "\n";
        std::cout << "Status: ";
        if (status == "pending") std::cout << YELLOW;
        else if (status == "completed") std::cout << GREEN;
        else if (status == "cancelled") std::cout << RED;
        std::cout << status << RESET << "\n";
        std::cout << "Total Amount: " << GREEN << "$" << total << RESET << "\n";
        std::cout << "Created: " << created << "\n";
        std::cout << "Shipping Address: " << shipping << "\n";

        auto items = extract_json_array(body, "items");
        if (!items.empty()) {
            std::cout << "\n" << BOLD << "Order Items:" << RESET << "\n";
            std::cout << "┌────────────┬──────────┬─────────────┬──────────────┐\n";
            std::cout << "│ Product ID │ Quantity │ Unit Price  │ Subtotal     │\n";
            std::cout << "├────────────┼──────────┼─────────────┼──────────────┤\n";

            for (const auto& item : items) {
                std::string prod_id = extract_json_value(item, "product_id");
                std::string qty = extract_json_value(item, "quantity");
                std::string price = extract_json_value(item, "price");

                double qty_val = std::stod(qty);
                double price_val = std::stod(price);
                double subtotal = qty_val * price_val;

                std::cout << "│ " << std::setw(10) << std::left << prod_id << " │ "
                          << std::setw(8) << std::right << qty << " │ "
                          << "$" << std::setw(10) << std::right << std::fixed << std::setprecision(2) << price << " │ "
                          << "$" << std::setw(11) << std::right << subtotal << " │\n";
            }
            std::cout << "└────────────┴──────────┴─────────────┴──────────────┘\n";
        }
    }

    void run() {
        if (!connect_to_server()) {
            std::cout << RED << "[ERROR] Failed to connect to server "
                      << server_ip_ << ":" << server_port_ << RESET << "\n";
            std::cout << "Make sure the server is running!\n";
            return;
        }
        disconnect();

        std::cout << HIDE_CURSOR;

        while (true) {
            display_dashboard();

            if (refresh_interval_ > 0) {
                std::cout << "\n" << YELLOW << "Auto-refreshing in " << refresh_interval_
                          << " seconds... (press any key for menu)" << RESET << std::flush;

                // Simple non-blocking wait
                auto start = std::chrono::steady_clock::now();
                while (true) {
                    auto now = std::chrono::steady_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start);
                    if (elapsed.count() >= refresh_interval_) {
                        break;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
                continue;
            }

            std::cout << "\nChoice: ";
            std::cout << SHOW_CURSOR;

            char choice;
            std::cin >> choice;
            std::cin.ignore();

            std::cout << HIDE_CURSOR;

            switch (tolower(choice)) {
                case '1':
                    view_all_users();
                    std::cout << "\nPress Enter to continue...";
                    std::cout << SHOW_CURSOR;
                    std::cin.get();
                    std::cout << HIDE_CURSOR;
                    break;
                case '2':
                    view_all_products();
                    std::cout << "\nPress Enter to continue...";
                    std::cout << SHOW_CURSOR;
                    std::cin.get();
                    std::cout << HIDE_CURSOR;
                    break;
                case '3':
                    update_order_status();
                    std::cout << "\nPress Enter to continue...";
                    std::cout << SHOW_CURSOR;
                    std::cin.ignore();
                    std::cin.get();
                    std::cout << HIDE_CURSOR;
                    break;
                case '4':
                    view_order_details();
                    std::cout << "\nPress Enter to continue...";
                    std::cout << SHOW_CURSOR;
                    std::cin.ignore();
                    std::cin.get();
                    std::cout << HIDE_CURSOR;
                    break;
                case 'r':
                    // Just refresh - loop will continue
                    break;
                case 'q':
                    std::cout << SHOW_CURSOR << CLEAR_SCREEN;
                    std::cout << GREEN << "\nThank you for using Admin Dashboard!\n" << RESET;
                    return;
                default:
                    std::cout << RED << "Invalid choice!" << RESET << "\n";
                    std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
};

void print_usage(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " [OPTIONS] <server_ip> <port>\n\n";
    std::cout << "Options:\n";
    std::cout << "  --server <ip>      Server IP address (default: 127.0.0.1)\n";
    std::cout << "  --port <port>      Server port (default: 8080)\n";
    std::cout << "  --refresh <sec>    Auto-refresh interval in seconds (0 = manual, default: 0)\n";
    std::cout << "  -h, --help         Show this help message\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << prog_name << " 192.168.1.100 8080\n";
    std::cout << "  " << prog_name << " --server 10.0.0.5 --port 8080 --refresh 5\n";
    std::cout << "  " << prog_name << " localhost 8080 --refresh 10\n";
}

int main(int argc, char** argv) {
    std::string server_ip = "127.0.0.1";
    int server_port = 8080;
    int refresh_interval = 0;

    // Parse arguments
    int i = 1;
    while (i < argc) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "--server") {
            if (i + 1 < argc) {
                server_ip = argv[++i];
            } else {
                std::cerr << RED << "Error: --server requires an argument" << RESET << "\n";
                return 1;
            }
        } else if (arg == "--port") {
            if (i + 1 < argc) {
                server_port = std::atoi(argv[++i]);
            } else {
                std::cerr << RED << "Error: --port requires an argument" << RESET << "\n";
                return 1;
            }
        } else if (arg == "--refresh") {
            if (i + 1 < argc) {
                refresh_interval = std::atoi(argv[++i]);
            } else {
                std::cerr << RED << "Error: --refresh requires an argument" << RESET << "\n";
                return 1;
            }
        } else {
            // Positional arguments
            if (arg[0] != '-') {
                server_ip = arg;
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    server_port = std::atoi(argv[++i]);
                }
            }
        }
        i++;
    }

    std::cout << BOLD << CYAN;
    std::cout << "╔════════════════════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                                                ║\n";
    std::cout << "║              " << GREEN << "REST API ADMIN DASHBOARD CLIENT" << CYAN << "                               ║\n";
    std::cout << "║                                                                                ║\n";
    std::cout << "║              " << WHITE << "Remote Server Monitoring & Management" << CYAN << "                          ║\n";
    std::cout << "║                                                                                ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════════════════════╝\n";
    std::cout << RESET << "\n";

    std::cout << "Connecting to: " << CYAN << server_ip << ":" << server_port << RESET << "\n";
    if (refresh_interval > 0) {
        std::cout << "Auto-refresh: " << GREEN << "Enabled (" << refresh_interval << "s)" << RESET << "\n";
    } else {
        std::cout << "Auto-refresh: " << YELLOW << "Disabled (manual mode)" << RESET << "\n";
    }
    std::cout << "\n";

    AdminDashboard dashboard(server_ip, server_port, refresh_interval);
    dashboard.run();

    return 0;
}
