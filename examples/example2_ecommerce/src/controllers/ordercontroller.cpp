#include "controllers/ordercontroller.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

void OrderController::setRawRequest(const std::string& raw) {
    raw_request = raw;
}

std::string OrderController::extractBody(const std::string& raw_req) {
    // Find body after \r\n\r\n
    size_t pos = raw_req.find("\r\n\r\n");
    if (pos == std::string::npos) {
        return "";
    }
    return raw_req.substr(pos + 4);
}

std::string OrderController::jsonResponse(int status, const std::string& body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << status;

    switch (status) {
        case 200: response << " OK"; break;
        case 201: response << " Created"; break;
        case 204: response << " No Content"; break;
        case 400: response << " Bad Request"; break;
        case 401: response << " Unauthorized"; break;
        case 403: response << " Forbidden"; break;
        case 404: response << " Not Found"; break;
        case 500: response << " Internal Server Error"; break;
        default: response << " Unknown"; break;
    }

    response << "\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}

int OrderController::extractUserId(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    // In production, extract from JWT token
    // For now, check params for user_id
    auto it = params.find("user_id");
    if (it != params.end()) {
        return std::stoi(it->second);
    }

    // Check target for query string (e.g., /api/orders?user_id=123)
    std::string target = req.getTarget();
    std::string search = "user_id=";
    size_t pos = target.find(search);
    if (pos != std::string::npos) {
        pos += search.length();
        size_t end = target.find("&", pos);
        std::string user_id_str = (end == std::string::npos) ?
            target.substr(pos) : target.substr(pos, end - pos);
        return std::stoi(user_id_str);
    }

    // Default to user 1 for testing
    return 1;
}

bool OrderController::isAdmin(int user_id) {
    // In production, check user role from database or JWT
    // For now, user_id 1 is admin
    return user_id == 1;
}

// POST /api/orders - Create new order
std::string OrderController::createOrder(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[OrderController] POST /api/orders\n";
    
    try {
        // Extract user_id (from authentication)
        int user_id = extractUserId(req, params);
        
        // Extract body
        std::string body = extractBody(raw_request);
        std::cout << "[OrderController] Body: " << body << "\n";
        
        if (body.empty()) {
            return jsonResponse(400, "{\"error\":\"Request body is required\"}");
        }
        
        // Parse order from JSON
        Order order = Order::fromJson(body);
        
        if (order.items.empty()) {
            return jsonResponse(400, "{\"error\":\"Order must have at least one item\"}");
        }
        
        // Create order
        Order created = service.createOrder(user_id, order.items, 
                                           order.shipping_address, order.notes);
        
        return jsonResponse(201, created.toJson());
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

// GET /api/orders - List orders
std::string OrderController::getOrders(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[OrderController] GET /api/orders\n";
    
    try {
        int user_id = extractUserId(req, params);
        bool admin = isAdmin(user_id);
        
        std::vector<Order> orders;
        
        if (admin) {
            // Admin sees all orders
            orders = service.getOrders();
        } else {
            // Regular user sees only their orders
            orders = service.getUserOrders(user_id);
        }
        
        // Build JSON array
        std::ostringstream json;
        json << "[";
        for (size_t i = 0; i < orders.size(); ++i) {
            json << orders[i].toJson();
            if (i < orders.size() - 1) {
                json << ",";
            }
        }
        json << "]";
        
        return jsonResponse(200, json.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

// GET /api/orders/:id - Get order by ID
std::string OrderController::getOrderById(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[OrderController] GET /api/orders/:id\n";
    
    try {
        // Extract ID from params
        auto it = params.find("id");
        if (it == params.end()) {
            return jsonResponse(400, "{\"error\":\"Order ID is required\"}");
        }
        
        int order_id = std::stoi(it->second);
        int user_id = extractUserId(req, params);
        bool admin = isAdmin(user_id);
        
        // Validate access
        if (!admin) {
            service.validateOrderAccess(order_id, user_id, false);
        }
        
        auto order = service.getOrder(order_id);
        
        if (!order.has_value()) {
            std::ostringstream error;
            error << "{\"error\":\"Order with ID " << order_id << " not found\"}";
            return jsonResponse(404, error.str());
        }
        
        return jsonResponse(200, order->toJson());
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

// PUT /api/orders/:id/status - Update order status
std::string OrderController::updateOrderStatus(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[OrderController] PUT /api/orders/:id/status\n";
    
    try {
        int user_id = extractUserId(req, params);
        
        // Only admin can update status
        if (!isAdmin(user_id)) {
            return jsonResponse(403, "{\"error\":\"Only administrators can update order status\"}");
        }
        
        // Extract ID
        auto it = params.find("id");
        if (it == params.end()) {
            return jsonResponse(400, "{\"error\":\"Order ID is required\"}");
        }
        int order_id = std::stoi(it->second);
        
        // Extract body
        std::string body = extractBody(raw_request);
        if (body.empty()) {
            return jsonResponse(400, "{\"error\":\"Request body is required\"}");
        }
        
        // Extract status from JSON
        std::string status;
        std::string search = "\"status\":\"";
        size_t pos = body.find(search);
        if (pos != std::string::npos) {
            pos += search.length();
            size_t end = body.find("\"", pos);
            if (end != std::string::npos) {
                status = body.substr(pos, end - pos);
            }
        }
        
        if (status.empty()) {
            return jsonResponse(400, "{\"error\":\"Status is required\"}");
        }
        
        // Update status
        service.updateOrderStatus(order_id, status);
        
        // Get updated order
        auto updated = service.getOrder(order_id);
        if (updated.has_value()) {
            return jsonResponse(200, updated->toJson());
        } else {
            return jsonResponse(200, "{\"message\":\"Order status updated successfully\"}");
        }
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

// DELETE /api/orders/:id - Cancel order
std::string OrderController::cancelOrder(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[OrderController] DELETE /api/orders/:id\n";
    
    try {
        // Extract ID
        auto it = params.find("id");
        if (it == params.end()) {
            return jsonResponse(400, "{\"error\":\"Order ID is required\"}");
        }
        int order_id = std::stoi(it->second);
        
        int user_id = extractUserId(req, params);
        bool admin = isAdmin(user_id);
        
        // Validate access (user can only cancel their own orders unless admin)
        if (!admin) {
            service.validateOrderAccess(order_id, user_id, false);
        }
        
        // Cancel order
        service.cancelOrder(order_id);
        
        return jsonResponse(200, "{\"message\":\"Order cancelled successfully\"}");
    } catch (const std::invalid_argument& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(400, error.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}

// GET /api/orders/stats - Get order statistics
std::string OrderController::getStatistics(const HttpRequest& req, const std::map<std::string, std::string>& params) {
    std::cout << "[OrderController] GET /api/orders/stats\n";
    
    try {
        int user_id = extractUserId(req, params);
        
        // Only admin can view statistics
        if (!isAdmin(user_id)) {
            return jsonResponse(403, "{\"error\":\"Only administrators can view order statistics\"}");
        }
        
        auto stats = service.getOrderStatistics();
        
        // Build JSON response
        std::ostringstream json;
        json << "{";
        json << "\"total_orders\":" << stats.total_orders << ",";
        json << "\"total_revenue\":" << std::fixed << std::setprecision(2) << stats.total_revenue << ",";
        json << "\"pending_orders\":" << stats.pending_orders << ",";
        json << "\"completed_orders\":" << stats.completed_orders << ",";
        json << "\"cancelled_orders\":" << stats.cancelled_orders;
        json << "}";
        
        return jsonResponse(200, json.str());
    } catch (const std::exception& e) {
        std::ostringstream error;
        error << "{\"error\":\"" << e.what() << "\"}";
        return jsonResponse(500, error.str());
    }
}
