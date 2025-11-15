#include "services/orderservice.hpp"
#include <iostream>
#include <stdexcept>
#include <iomanip>
#include <sstream>

// Helper to calculate total from items
double OrderService::calculateTotal(const std::vector<OrderItem>& items) {
    double total = 0.0;
    for (const auto& item : items) {
        total += item.price_at_purchase * item.quantity;
    }
    return total;
}

// Validate order items
void OrderService::validateOrderItems(const std::vector<OrderItem>& items) {
    if (items.empty()) {
        throw std::invalid_argument("Order must have at least one item");
    }

    for (const auto& item : items) {
        // Check if product exists
        auto product = productRepository.getById(item.product_id);
        if (!product.has_value()) {
            throw std::invalid_argument("Product with ID " + std::to_string(item.product_id) + " does not exist");
        }

        // Check if product is active
        if (!product->is_active) {
            throw std::invalid_argument("Product '" + product->name + "' is not available");
        }

        // Check stock availability
        if (product->stock_quantity < item.quantity) {
            throw std::invalid_argument("Insufficient stock for product '" + product->name + 
                                        "'. Available: " + std::to_string(product->stock_quantity) + 
                                        ", Requested: " + std::to_string(item.quantity));
        }

        // Validate quantity
        if (item.quantity <= 0) {
            throw std::invalid_argument("Item quantity must be greater than 0");
        }

        // Validate price
        if (item.price_at_purchase < 0) {
            throw std::invalid_argument("Item price cannot be negative");
        }
    }
}

// Decrement product stock after order is placed
void OrderService::decrementProductStock(const std::vector<OrderItem>& items) {
    for (const auto& item : items) {
        auto product = productRepository.getById(item.product_id);
        if (product.has_value()) {
            int new_stock = product->stock_quantity - item.quantity;
            if (new_stock < 0) {
                throw std::runtime_error("Stock calculation error for product ID " + 
                                         std::to_string(item.product_id));
            }
            productRepository.updateStock(item.product_id, new_stock);
        }
    }
}

// Increment product stock when order is cancelled
void OrderService::incrementProductStock(const std::vector<OrderItem>& items) {
    for (const auto& item : items) {
        auto product = productRepository.getById(item.product_id);
        if (product.has_value()) {
            int new_stock = product->stock_quantity + item.quantity;
            productRepository.updateStock(item.product_id, new_stock);
        }
    }
}

// Create a new order
Order OrderService::createOrder(int user_id, const std::vector<OrderItem>& items,
                                const std::string& shipping_address, const std::string& notes) {
    std::cout << "[OrderService] createOrder for user_id=" << user_id << "\n";

    // Validate user_id
    if (user_id <= 0) {
        throw std::invalid_argument("Invalid user ID");
    }

    // Validate shipping address
    if (shipping_address.empty()) {
        throw std::invalid_argument("Shipping address is required");
    }

    // Validate order items
    validateOrderItems(items);

    // Create order object
    Order order;
    order.user_id = user_id;
    order.status = "pending";
    order.shipping_address = shipping_address;
    order.notes = notes;
    order.total_amount = calculateTotal(items);

    // Save order to database
    Order created_order = orderRepository.create(order);

    // Add order items
    for (auto item : items) {
        item.order_id = created_order.id;
        orderRepository.addOrderItem(item);
        created_order.items.push_back(item);
    }

    // Decrement product stock
    decrementProductStock(items);

    std::cout << "[OrderService] Order created successfully with ID=" << created_order.id << "\n";
    return created_order;
}

// Get all orders
std::vector<Order> OrderService::getOrders() {
    std::cout << "[OrderService] getOrders()\n";
    return orderRepository.getAll();
}

// Get order by ID
std::optional<Order> OrderService::getOrder(int id) {
    std::cout << "[OrderService] getOrder(" << id << ")\n";
    
    if (id <= 0) {
        throw std::invalid_argument("Invalid order ID");
    }
    
    return orderRepository.getOrderWithItems(id);
}

// Get orders for a specific user
std::vector<Order> OrderService::getUserOrders(int user_id) {
    std::cout << "[OrderService] getUserOrders(" << user_id << ")\n";
    
    if (user_id <= 0) {
        throw std::invalid_argument("Invalid user ID");
    }
    
    return orderRepository.getUserOrdersWithItems(user_id);
}

// Update order status
void OrderService::updateOrderStatus(int id, const std::string& status) {
    std::cout << "[OrderService] updateOrderStatus(" << id << ", " << status << ")\n";
    
    if (id <= 0) {
        throw std::invalid_argument("Invalid order ID");
    }
    
    validateOrderStatus(status);
    
    // Check if order exists
    auto order = orderRepository.getById(id);
    if (!order.has_value()) {
        throw std::invalid_argument("Order with ID " + std::to_string(id) + " does not exist");
    }
    
    // Don't allow changing status of cancelled orders
    if (order->status == "cancelled") {
        throw std::invalid_argument("Cannot change status of cancelled order");
    }
    
    orderRepository.updateStatus(id, status);
}

// Cancel order
void OrderService::cancelOrder(int id) {
    std::cout << "[OrderService] cancelOrder(" << id << ")\n";
    
    if (id <= 0) {
        throw std::invalid_argument("Invalid order ID");
    }
    
    // Get order with items
    auto order = orderRepository.getOrderWithItems(id);
    if (!order.has_value()) {
        throw std::invalid_argument("Order with ID " + std::to_string(id) + " does not exist");
    }
    
    // Check if already cancelled
    if (order->status == "cancelled") {
        throw std::invalid_argument("Order is already cancelled");
    }
    
    // Can't cancel shipped or delivered orders
    if (order->status == "shipped" || order->status == "delivered") {
        throw std::invalid_argument("Cannot cancel " + order->status + " order");
    }
    
    // Restore product stock
    incrementProductStock(order->items);
    
    // Update status to cancelled
    orderRepository.updateStatus(id, "cancelled");
}

// Get order statistics
OrderService::OrderStatistics OrderService::getOrderStatistics() {
    std::cout << "[OrderService] getOrderStatistics()\n";
    
    OrderStatistics stats;
    stats.total_orders = orderRepository.getTotalOrderCount();
    stats.total_revenue = orderRepository.getTotalRevenue();
    
    auto pending = orderRepository.getByStatus("pending");
    stats.pending_orders = pending.size();
    
    auto completed = orderRepository.getByStatus("delivered");
    stats.completed_orders = completed.size();
    
    auto cancelled = orderRepository.getByStatus("cancelled");
    stats.cancelled_orders = cancelled.size();
    
    return stats;
}

// Validate order status
void OrderService::validateOrderStatus(const std::string& status) {
    if (status != "pending" && status != "processing" && 
        status != "shipped" && status != "delivered" && status != "cancelled") {
        throw std::invalid_argument("Invalid order status. Must be one of: pending, processing, shipped, delivered, cancelled");
    }
}

// Validate order access (user can only access their own orders unless admin)
void OrderService::validateOrderAccess(int order_id, int user_id, bool is_admin) {
    auto order = orderRepository.getById(order_id);
    if (!order.has_value()) {
        throw std::invalid_argument("Order with ID " + std::to_string(order_id) + " does not exist");
    }
    
    if (!is_admin && order->user_id != user_id) {
        throw std::invalid_argument("Access denied: You can only view your own orders");
    }
}
