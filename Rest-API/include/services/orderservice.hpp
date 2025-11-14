#pragma once
#include "data/orderrepository.hpp"
#include "data/productrepository.hpp"
#include "models/order.hpp"
#include <vector>
#include <string>
#include <optional>
#include <map>

class OrderService {
private:
    OrderRepository& orderRepository;
    ProductRepository& productRepository;

    // Helper methods
    double calculateTotal(const std::vector<OrderItem>& items);
    void validateOrderItems(const std::vector<OrderItem>& items);
    void decrementProductStock(const std::vector<OrderItem>& items);
    void incrementProductStock(const std::vector<OrderItem>& items);

public:
    OrderService(OrderRepository& orderRepo, ProductRepository& productRepo) 
        : orderRepository(orderRepo), productRepository(productRepo) {}

    // Order management
    Order createOrder(int user_id, const std::vector<OrderItem>& items, 
                      const std::string& shipping_address, const std::string& notes = "");
    std::vector<Order> getOrders();
    std::optional<Order> getOrder(int id);
    std::vector<Order> getUserOrders(int user_id);
    void updateOrderStatus(int id, const std::string& status);
    void cancelOrder(int id);

    // Statistics
    struct OrderStatistics {
        int total_orders;
        double total_revenue;
        int pending_orders;
        int completed_orders;
        int cancelled_orders;
    };
    OrderStatistics getOrderStatistics();

    // Validation
    void validateOrderStatus(const std::string& status);
    void validateOrderAccess(int order_id, int user_id, bool is_admin = false);
};
