#pragma once
#include "data/databaseconnection.hpp"
#include "models/order.hpp"
#include <string>
#include <vector>
#include <optional>

class OrderRepository {
    DatabaseConnection& conn;
public:
    explicit OrderRepository(DatabaseConnection& c): conn(c) {}

    // CRUD operations for orders
    std::vector<Order> getAll();
    std::optional<Order> getById(int id);
    std::vector<Order> getByUserId(int user_id);
    Order create(const Order& order);
    void update(const Order& order);
    void updateStatus(int id, const std::string& status);
    void deleteById(int id);

    // Order items operations
    std::vector<OrderItem> getOrderItems(int order_id);
    OrderItem addOrderItem(const OrderItem& item);
    void deleteOrderItems(int order_id);

    // Query methods
    std::vector<Order> getByStatus(const std::string& status);
    int getTotalOrderCount();
    double getTotalRevenue();
    
    // Transaction support - get order with all items
    std::optional<Order> getOrderWithItems(int id);
    std::vector<Order> getUserOrdersWithItems(int user_id);
};
