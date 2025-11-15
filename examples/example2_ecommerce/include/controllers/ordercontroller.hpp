#pragma once
#include "services/orderservice.hpp"
#include "http/request.hpp"
#include <string>
#include <map>

class OrderController {
private:
    OrderService& service;
    
    // Helper to extract body from request
    std::string extractBody(const std::string& raw_request);
    
    // Helper to build HTTP JSON response
    std::string jsonResponse(int status, const std::string& body);

    // Helper to extract user_id from request (from authentication token/session)
    // For now, we'll extract it from query params or body
    int extractUserId(const HttpRequest& req, const std::map<std::string, std::string>& params);
    
    // Helper to check if user is admin (for now, hardcoded - in production use JWT)
    bool isAdmin(int user_id);

public:
    explicit OrderController(OrderService& service) : service(service) {}
    
    // POST /api/orders - Create new order (authenticated users)
    std::string createOrder(const HttpRequest& req, const std::map<std::string, std::string>& params);
    
    // GET /api/orders - List user's orders (or all for admin)
    std::string getOrders(const HttpRequest& req, const std::map<std::string, std::string>& params);
    
    // GET /api/orders/:id - Get order details with items
    std::string getOrderById(const HttpRequest& req, const std::map<std::string, std::string>& params);
    
    // PUT /api/orders/:id/status - Update order status (admin only)
    std::string updateOrderStatus(const HttpRequest& req, const std::map<std::string, std::string>& params);
    
    // DELETE /api/orders/:id - Cancel order
    std::string cancelOrder(const HttpRequest& req, const std::map<std::string, std::string>& params);
    
    // GET /api/orders/stats - Order statistics (admin only)
    std::string getStatistics(const HttpRequest& req, const std::map<std::string, std::string>& params);

    // Set raw request for parsing body
    void setRawRequest(const std::string& raw);
    
private:
    std::string raw_request;
};
